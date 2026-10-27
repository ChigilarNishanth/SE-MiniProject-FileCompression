#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using Frequencies = std::array<uint64_t, 256>;
constexpr uint64_t HEADER_SIZE = 2072;
constexpr uint64_t DEFAULT_LIMIT = 1024ULL * 1024 * 1024;

struct Failure : std::runtime_error { using std::runtime_error::runtime_error; };
void require(bool ok, const char* message) { if (!ok) throw Failure(message); }

uint32_t crc_update(uint32_t crc, unsigned char b) {
    static const auto table = [] {
        std::array<uint32_t,256> t{};
        for (uint32_t i=0; i<256; ++i) {
            uint32_t v=i;
            for (int j=0; j<8; ++j) v=(v>>1) ^ ((v&1) ? 0xedb88320U : 0U);
            t[i]=v;
        }
        return t;
    }();
    return (crc>>8) ^ table[(crc^b)&255];
}

// A unique sibling directory owns all provisional data. Publishing uses an
// atomic, no-replace hard link, so existing destinations are never overwritten.
struct Output {
    fs::path target, directory, temporary;
    std::ofstream stream;
    explicit Output(const fs::path& destination) : target(destination) {
        require(!fs::exists(target), "OUTPUT_EXISTS: destination already exists");
        auto parent=target.parent_path();
        if (parent.empty()) parent=".";
        const auto stamp=std::chrono::steady_clock::now().time_since_epoch().count();
        bool created=false;
        for (int i=0; i<100; ++i) {
            directory=parent/(".czip-tmp-"+std::to_string(stamp)+"-"+std::to_string(i));
            std::error_code ec;
            if (fs::create_directory(directory,ec)) { created=true; break; }
            require(!ec || ec==std::errc::file_exists, "IO_ERROR: cannot create temporary directory");
        }
        require(created,"IO_ERROR: cannot reserve temporary directory");
        temporary=directory/"data";
        stream.open(temporary,std::ios::binary);
        if (!stream) { std::error_code ec; fs::remove_all(directory,ec); throw Failure("IO_ERROR: cannot create output"); }
    }
    void commit() {
        stream.flush();
        require(bool(stream),"IO_ERROR: output write failed");
        stream.close();
        require(!stream.fail(),"IO_ERROR: output close failed");
        std::error_code ec;
        fs::create_hard_link(temporary,target,ec);
        require(!ec,"IO_ERROR: cannot publish output (destination exists or filesystem lacks hard links)");
    }
    ~Output() { stream.close(); std::error_code ec; if (!directory.empty()) fs::remove_all(directory,ec); }
};

struct Node { uint64_t count; int minimum, symbol; Node *left, *right; };
struct Tree {
    std::vector<std::unique_ptr<Node>> nodes;
    Node* root=nullptr;
    explicit Tree(const Frequencies& frequencies) {
        auto compare=[](Node* a,Node* b) { return a->count!=b->count ? a->count>b->count : a->minimum>b->minimum; };
        std::priority_queue<Node*,std::vector<Node*>,decltype(compare)> heap(compare);
        auto add=[&](uint64_t n,int m,int s,Node* l,Node* r) {
            nodes.push_back(std::make_unique<Node>(Node{n,m,s,l,r})); return nodes.back().get();
        };
        for (int i=0;i<256;++i) if (frequencies[i]) heap.push(add(frequencies[i],i,i,nullptr,nullptr));
        while (heap.size()>1) {
            auto a=heap.top(); heap.pop(); auto b=heap.top(); heap.pop();
            heap.push(add(a->count+b->count,std::min(a->minimum,b->minimum),-1,a,b));
        }
        if (!heap.empty()) root=heap.top();
    }
    void codes(Node* n,std::vector<bool>& path,std::array<std::vector<bool>,256>& result) const {
        if (!n) return;
        if (n->symbol>=0) { result[n->symbol]=path.empty()?std::vector<bool>{false}:path; return; }
        path.push_back(false); codes(n->left,path,result);
        path.back()=true; codes(n->right,path,result); path.pop_back();
    }
    auto codebook() const {
        std::array<std::vector<bool>,256> result; std::vector<bool> path;
        codes(root,path,result); return result;
    }
};

void put(std::ostream& s,uint64_t v,int bytes) { for(int i=0;i<bytes;++i) s.put(static_cast<char>((v>>(8*i))&255)); }
uint64_t get(std::istream& s,int bytes) {
    uint64_t v=0;
    for(int i=0;i<bytes;++i) { int b=s.get(); require(b!=EOF,"SEC_ERR_BAD_HEADER: truncated header"); v|=uint64_t(b)<<(8*i); }
    return v;
}
uint64_t bit_count(const Frequencies& f,const std::array<std::vector<bool>,256>& codes) {
    uint64_t bits=0;
    for(int i=0;i<256;++i) {
        const auto length=codes[i].size();
        require(!length || f[i] <= (std::numeric_limits<uint64_t>::max()-bits)/length,"SEC_ERR_BAD_HEADER: bit count overflow");
        bits+=f[i]*length;
    }
    return bits;
}
struct Stats { uint64_t original, archive; };

Stats compress(const fs::path& source,const fs::path& destination) {
    std::ifstream input(source,std::ios::binary);
    require(bool(input),"IO_ERROR: cannot open input");
    Frequencies freq{}; uint64_t size=0; uint32_t crc=0xffffffffU;
    std::array<char,65536> buffer{};
    while(input.read(buffer.data(),buffer.size()) || input.gcount()) {
        for(std::streamsize i=0;i<input.gcount();++i) {
            auto c=static_cast<unsigned char>(buffer[i]); ++freq[c]; crc=crc_update(crc,c); ++size;
        }
    }
    require(input.eof(),"IO_ERROR: input read failed");
    Tree tree(freq); auto codes=tree.codebook(); const uint64_t bits=bit_count(freq,codes);
    input.clear(); input.seekg(0); require(bool(input),"IO_ERROR: input is not seekable");
    Output output(destination); auto& out=output.stream;
    out.write("CZ",2); put(out,1,1); put(out,0,1); put(out,size,8); put(out,bits,8); put(out,crc^0xffffffffU,4);
    for(auto n:freq) put(out,n,8);
    uint8_t packed=0; int used=0; Frequencies observed{}; uint32_t second_crc=0xffffffffU; uint64_t seen=0;
    while(input.read(buffer.data(),buffer.size()) || input.gcount()) {
        for(std::streamsize i=0;i<input.gcount();++i) {
            auto c=static_cast<unsigned char>(buffer[i]); ++observed[c]; ++seen; second_crc=crc_update(second_crc,c);
            for(bool bit:codes[c]) {
                packed=static_cast<uint8_t>((packed<<1)|bit);
                if(++used==8) { out.put(static_cast<char>(packed)); packed=0; used=0; }
            }
        }
    }
    require(input.eof(),"IO_ERROR: input read failed");
    require(seen==size && observed==freq && second_crc==crc,"IO_ERROR: input changed during compression");
    if(used) out.put(static_cast<char>(packed<<(8-used)));
    output.commit(); return {size,HEADER_SIZE+bits/8+(bits%8!=0)};
}

Stats decompress(const fs::path& source,const fs::path& destination,uint64_t limit) {
    std::ifstream input(source,std::ios::binary);
    require(bool(input),"IO_ERROR: cannot open input");
    require(get(input,1)=='C' && get(input,1)=='Z',"SEC_ERR_BAD_HEADER: invalid magic");
    require(get(input,1)==1 && get(input,1)==0,"SEC_ERR_BAD_HEADER: unsupported version or flags");
    const uint64_t size=get(input,8), bits=get(input,8); const auto expected_crc=get(input,4);
    require(size<=limit,"SEC_ERR_BAD_HEADER: declared output exceeds --max-output limit");
    Frequencies freq{}; uint64_t total=0;
    for(auto& n:freq) { n=get(input,8); require(n<=size-total,"SEC_ERR_BAD_HEADER: invalid frequencies"); total+=n; }
    require(total==size,"SEC_ERR_BAD_HEADER: frequency total differs from size");
    Tree tree(freq); const auto codes=tree.codebook();
    require(bit_count(freq,codes)==bits,"SEC_ERR_BAD_HEADER: inconsistent bit count");
    input.seekg(0,std::ios::end); const auto end=input.tellg();
    const uint64_t archive_size=HEADER_SIZE+bits/8+(bits%8!=0);
    require(end>=0 && static_cast<uint64_t>(end)==archive_size,"SEC_ERR_MALFORMED_STREAM: truncated or trailing data");
    input.seekg(HEADER_SIZE);
    Output output(destination); Frequencies observed{}; uint64_t written=0; uint32_t crc=0xffffffffU;
    Node* node=tree.root;
    for(uint64_t offset=0;offset<bits;) {
        const int byte=input.get(); require(byte!=EOF,"SEC_ERR_MALFORMED_STREAM: truncated payload");
        const int count=static_cast<int>(std::min<uint64_t>(8,bits-offset));
        for(int j=0;j<count;++j) {
            const bool bit=(byte>>(7-j))&1;
            require(node!=nullptr,"SEC_ERR_MALFORMED_STREAM: missing tree");
            if(tree.root->symbol>=0) require(!bit,"SEC_ERR_MALFORMED_STREAM: invalid single-symbol code");
            else node=bit?node->right:node->left;
            require(node!=nullptr,"SEC_ERR_MALFORMED_STREAM: invalid traversal");
            if(node->symbol>=0) {
                const auto symbol=static_cast<unsigned char>(node->symbol);
                require(written<size && observed[symbol]<freq[symbol],"SEC_ERR_MALFORMED_STREAM: decoded frequency overflow");
                output.stream.put(static_cast<char>(symbol)); ++observed[symbol]; ++written; crc=crc_update(crc,symbol); node=tree.root;
            }
        }
        require(count==8 || (byte&((1<<(8-count))-1))==0,"SEC_ERR_MALFORMED_STREAM: nonzero padding");
        offset+=count;
    }
    require(node==tree.root && written==size && observed==freq && (crc^0xffffffffU)==expected_crc,
            "SEC_ERR_MALFORMED_STREAM: size, frequency, or CRC-32 integrity check failed");
    output.commit(); return {size,archive_size};
}

void usage(std::ostream& out) {
    out<<"czip - lossless Huffman compression (custom .czip format)\n"
       <<"Usage: czip -c INPUT [OUTPUT]\n"
       <<"       czip -d INPUT.czip [OUTPUT] [--max-output BYTES]\n"
       <<"       czip --help\n"
       <<"Defaults: INPUT.czip for compression; remove .czip or append .out for decompression.\n"
       <<"Existing output files are never replaced. Default decompression limit: 1073741824 bytes.\n"
       <<"Exit codes: 0 success, 1 execution/integrity failure, 2 invalid syntax.\n";
}
int main(int argc,char** argv) {
    if(argc==2 && std::string(argv[1])=="--help") { usage(std::cout); return 0; }
    auto syntax=[] { usage(std::cerr); return 2; };
    if(argc<3 || (std::string(argv[1])!="-c" && std::string(argv[1])!="-d")) return syntax();
    const bool packing=std::string(argv[1])=="-c"; uint64_t limit=DEFAULT_LIMIT;
    fs::path source=argv[2], destination; bool have_output=false, have_limit=false;
    for(int i=3;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--max-output") {
            if(packing || have_limit || ++i>=argc) return syntax();
            const std::string number=argv[i];
            if(number.empty() || number.find_first_not_of("0123456789")!=std::string::npos) return syntax();
            try { limit=std::stoull(number); } catch(...) { return syntax(); }
            have_limit=true;
        } else {
            if(have_output || arg.rfind("--",0)==0) return syntax();
            destination=arg; have_output=true;
        }
    }
    if(!have_output) {
        destination=source;
        if(packing) destination+=".czip";
        else if(destination.extension()==".czip") destination.replace_extension();
        else destination+=".out";
    }
    try {
        require(fs::is_regular_file(source),"IO_ERROR: input must be a regular file");
        const auto start=std::chrono::steady_clock::now();
        const Stats stats=packing?compress(source,destination):decompress(source,destination,limit);
        const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
        std::cout<<"Original size: "<<stats.original<<" bytes\nArchive size: "<<stats.archive<<" bytes\nSavings: "
                 <<std::fixed<<std::setprecision(2)<<(stats.original?100.0*(1.0-double(stats.archive)/double(stats.original)):0.0)
                 <<"%\nExecution time: "<<ms<<" ms\nOutput: "<<destination.string()<<"\n";
        return 0;
    } catch(const std::exception& e) { std::cerr<<e.what()<<"\n"; return 1; }
}
