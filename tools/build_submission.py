"""Render the review package with native vector UML diagrams and paginated tables."""
from html import escape
from pathlib import Path
import re

from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.platypus import (SimpleDocTemplate, Paragraph, Spacer, PageBreak, Table, TableStyle,
                               KeepTogether)
from reportlab.graphics.shapes import Drawing, Rect, String, Line, Circle, Ellipse, Polygon
from reportlab.graphics import renderSVG

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / 'output/pdf'
OUTPUT.mkdir(parents=True, exist_ok=True)
DIAGRAMS = ROOT / 'docs/diagrams'
DIAGRAMS.mkdir(exist_ok=True)
INK = colors.HexColor('#163047')
TEAL = colors.HexColor('#087f80')
PALE = colors.HexColor('#edf6f7')
GRAY = colors.HexColor('#526477')
W = A4[0] - 104


def label(d, x, y, text, size=9, center=True, color=INK, font='Helvetica'):
    d.add(String(x, y, text, fontSize=size, textAnchor='middle' if center else 'start',
                 fillColor=color, fontName=font))


def arrow(d, x1, y1, x2, y2, dashed=False, color=TEAL):
    import math
    line = Line(x1, y1, x2, y2, strokeColor=color, strokeWidth=1)
    if dashed:
        line.strokeDashArray = [4, 3]
    d.add(line)
    angle = math.atan2(y2-y1, x2-x1)
    size = 5
    d.add(Polygon([x2, y2, x2-size*math.cos(angle-.5), y2-size*math.sin(angle-.5),
                   x2-size*math.cos(angle+.5), y2-size*math.sin(angle+.5)], fillColor=color, strokeColor=color))


def actor(d, x, y, name):
    d.add(Circle(x, y+22, 7, fillColor=colors.white, strokeColor=INK))
    for a, b, c, e in [(x,y+15,x,y-8), (x-12,y+5,x+12,y+5), (x,y-8,x-12,y-24), (x,y-8,x+12,y-24)]:
        d.add(Line(a,b,c,e,strokeColor=INK))
    label(d,x,y-40,name,8)


def usecase():
    d=Drawing(W,305)
    d.add(Rect(62,25,415,260,fillColor=colors.white,strokeColor=INK))
    label(d,270,267,'czip file compression utility',11,font='Helvetica-Bold')
    actor(d,24,174,'User')
    for x,y,lines in [(170,219,['UC-01','Compress file']), (170,107,['UC-02','Decompress file']),
                       (366,219,['UC-03','Display statistics']), (366,107,['UC-04','Validate integrity'])]:
        d.add(Ellipse(x,y,82,28,fillColor=PALE,strokeColor=TEAL))
        label(d,x,y+4,lines[0],9,font='Helvetica-Bold')
        label(d,x,y-10,lines[1],9)
    d.add(Line(36,180,88,219,strokeColor=INK))
    d.add(Line(36,165,88,107,strokeColor=INK))
    arrow(d,252,219,284,219,dashed=True)
    label(d,268,255,'<<include>>',8)
    arrow(d,252,107,284,107,dashed=True)
    label(d,270,143,'<<include>>',8)
    arrow(d,226,129,310,196,dashed=True)
    label(d,307,158,'<<include>>',8)
    label(d,269,8,'Host filesystem supplies input, staging and final publication.',9,color=GRAY)
    return d


def components():
    d=Drawing(W,315)
    def box(x,y,w,h,title,detail):
        d.add(Rect(x,y,w,h,fillColor=PALE,strokeColor=TEAL))
        label(d,x+w/2,y+h-14,'<<component>>',8,color=GRAY)
        label(d,x+w/2,y+h-30,title,10,font='Helvetica-Bold')
        label(d,x+w/2,y+12,detail,8)
    box(145,244,200,60,'CLI controller','main / usage / statistics')
    box(10,142,215,66,'Frequency + integrity model','256 counters / CRC-32 / Tree / codebook')
    box(266,142,215,66,'Archive codec','put / get / packed encode and decode')
    box(10,28,215,66,'Validation boundary','header / cap / length / padding / CRC')
    box(266,28,215,66,'Filesystem transaction','Output: stage / close / publish / cleanup')
    arrow(d,206,244,118,208)
    arrow(d,287,244,374,208)
    arrow(d,225,174,266,174)
    arrow(d,118,142,118,94)
    arrow(d,374,142,374,94)
    arrow(d,225,61,266,61)
    label(d,245,120,'validated metadata and payload before final publication',8,color=GRAY)
    return d


def sequence(decode=False):
    d=Drawing(W,355)
    positions=[35,151,298,444]
    names=['User','CLI','Validator' if decode else 'Model','Codec / storage']
    for x,name in zip(positions,names):
        d.add(Rect(x-40,314,80,27,fillColor=PALE,strokeColor=TEAL))
        label(d,x,324,name,9,font='Helvetica-Bold')
        line=Line(x,32,x,314,strokeColor=GRAY)
        line.strokeDashArray=[3,3]
        d.add(line)
    messages = ([(0,1,'-d archive output'),(1,2,'header + output cap'),(2,1,'validated frequencies'),
                 (1,3,'build tree; decode to staging'),(3,2,'size / frequency / CRC'),(2,3,'accept or reject'),
                 (3,1,'publish if valid; else discard'),(1,0,'statistics / failure')]
                if decode else
                [(0,1,'-c input archive'),(1,2,'count bytes + CRC'),(2,1,'tree + codebook'),
                 (1,3,'write header to staging'),(1,3,'second pass: encode bitstream'),(1,2,'verify source stable'),
                 (1,3,'flush, close, publish'),(1,0,'statistics / failure')])
    for i,(a,b,text) in enumerate(messages):
        y=289-30*i
        arrow(d,positions[a],y,positions[b],y,dashed=a>b)
        label(d,(positions[a]+positions[b])/2,y+6,f'{i+1}. {text}',8)
    label(d,W/2,15,'Failure: exit 1, no published destination, provisional output cleaned up.',8,color=GRAY)
    return d


drawings = {'usecase':usecase(), 'components':components(),
            'compress_sequence':sequence(), 'decompress_sequence':sequence(True)}
for name,drawing in drawings.items():
    renderSVG.drawToFile(drawing,str(DIAGRAMS/(name+'.svg')))

styles=getSampleStyleSheet()
styles.add(ParagraphStyle('BodyCustom',fontName='Helvetica',fontSize=10,leading=14,
                          textColor=INK,spaceAfter=8,splitLongWords=True))
styles.add(ParagraphStyle('CellCustom',fontName='Helvetica',fontSize=8.1,leading=10.8,textColor=INK,splitLongWords=True))
styles.add(ParagraphStyle('CellHeadCustom',parent=styles['CellCustom'],textColor=colors.white,fontName='Helvetica-Bold'))
styles.add(ParagraphStyle('TitleCustom',fontName='Helvetica-Bold',fontSize=28,leading=34,textColor=INK,spaceAfter=18))
styles.add(ParagraphStyle('H1Custom',fontName='Helvetica-Bold',fontSize=21,leading=26,textColor=INK,spaceAfter=14))
styles.add(ParagraphStyle('H2Custom',fontName='Helvetica-Bold',fontSize=13,leading=17,textColor=TEAL,spaceBefore=13,spaceAfter=8,keepWithNext=True))
styles.add(ParagraphStyle('H3Custom',fontName='Helvetica-Bold',fontSize=10.5,leading=14,textColor=INK,spaceBefore=9,spaceAfter=6,keepWithNext=True))
styles.add(ParagraphStyle('CoverSmall',fontName='Helvetica',fontSize=12,leading=18,textColor=GRAY,spaceAfter=14))


def markup(text):
    text=escape(text.strip())
    text=re.sub(r'\*\*(.*?)\*\*',r'<b>\1</b>',text)
    text=re.sub(r'`([^`]+)`',r'<font name="Courier">\1</font>',text)
    return text


def markdown_flowables(source):
    content=source.read_text()
    if source.name=='Backlog_Sprints.md':
        content=content.split('## Acceptance details')[0] + '\n\n## Acceptance records\nPer-story acceptance criteria, existing issue links, observed states and proposed ownership are included in docs/backlog.json and the full Markdown sprint plan. The plan does not close existing upstream issues.\n'
    lines=content.splitlines()
    result=[]; i=0
    while i<len(lines):
        line=lines[i].strip()
        if not line:
            i+=1; continue
        if line.startswith('|'):
            rows=[]
            while i<len(lines) and lines[i].strip().startswith('|'):
                cells=[c.strip() for c in lines[i].strip().strip('|').split('|')]
                if not all(re.fullmatch(r':?-+:?',c or ' ') for c in cells):
                    rows.append(cells)
                i+=1
            count=len(rows[0])
            if count==2:
                widths=[W*.24,W*.76]
            elif count==3:
                widths=[W*.13,W*.43,W*.44]
            elif count==4:
                widths=[W*.12,W*.27,W*.27,W*.34]
            elif count==5:
                widths=[W*.12,W*.12,W*.28,W*.18,W*.30]
            elif 'Story' in rows[0][0]:
                rows[0][1]='Pts'
                widths=[W*.28,W*.065,W*.085,W*.15,W*.16,W*.26]
                # Correct rounding/weights while keeping readable owner/evidence cells.
                widths=[v*W/sum(widths) for v in widths]
            else:
                widths=[W/count]*count
            cells=[[Paragraph(markup(c),styles['CellHeadCustom' if r==0 else 'CellCustom']) for c in row] for r,row in enumerate(rows)]
            table=Table(cells,colWidths=widths,repeatRows=1,hAlign='LEFT')
            table.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),INK),('VALIGN',(0,0),(-1,-1),'TOP'),
                                       ('LEFTPADDING',(0,0),(-1,-1),7),('RIGHTPADDING',(0,0),(-1,-1),7),
                                       ('TOPPADDING',(0,0),(-1,-1),7),('BOTTOMPADDING',(0,0),(-1,-1),7),
                                       ('ROWBACKGROUNDS',(0,1),(-1,-1),[colors.white,PALE]),
                                       ('LINEBELOW',(0,0),(-1,0),.5,TEAL),
                                       ('LINEBELOW',(0,1),(-1,-1),.25,colors.HexColor('#dbe5ec'))]))
            result.extend([table,Spacer(1,10)])
            continue
        match=re.fullmatch(r'!\[[^]]*\]\(diagrams/(\w+)\.svg\)',line)
        if match:
            result.extend([drawings[match[1]],Spacer(1,12)])
        elif line.startswith('### '):
            result.append(Paragraph(markup(line[4:]),styles['H3Custom']))
        elif line.startswith('## '):
            result.append(Paragraph(markup(line[3:]),styles['H2Custom']))
        elif line.startswith('# '):
            result.append(Paragraph(markup(line[2:]),styles['H1Custom']))
        elif re.match(r'^\d+\. ',line):
            result.append(Paragraph(markup(line),styles['BodyCustom']))
        else:
            paragraph=[line]
            while i+1<len(lines) and lines[i+1].strip() and not lines[i+1].strip().startswith(('#','|','![')) and not re.match(r'^\d+\. ',lines[i+1].strip()):
                i+=1; paragraph.append(lines[i].strip())
            result.append(Paragraph(markup(' '.join(paragraph)),styles['BodyCustom']))
        i+=1
    return result


story=[Spacer(1,50),Paragraph('SOFTWARE ENGINEERING<br/>MINI-PROJECT',styles['CoverSmall']),
       Paragraph('File Compression Tool',styles['TitleCustom']),
       Paragraph('Huffman coding in C++17<br/>Part 1 and Part 2 review package',styles['CoverSmall']),
       Spacer(1,22)]
team=[['Team E17','SRN'],['Mahesh Kumar B','PES1UG24CS255'],['Nishanth T Chigilar','PES1UG24CS302'],['Mudit Chaturvedi','PES1UG24CS279']]
team_table=Table([[Paragraph(c,styles['CellHeadCustom' if r==0 else 'BodyCustom']) for c in row] for r,row in enumerate(team)],colWidths=[W*.57,W*.43])
team_table.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),INK),('ROWBACKGROUNDS',(0,1),(-1,-1),[PALE,colors.white]),
                                ('VALIGN',(0,0),(-1,-1),'TOP'),('TOPPADDING',(0,0),(-1,-1),10),('BOTTOMPADDING',(0,0),(-1,-1),9)]))
story += [team_table,Spacer(1,28),Paragraph('Prepared 27 October 2026 | Revision 2.0',styles['CoverSmall']),
          Paragraph('Review candidate: local implementation and measured evidence are complete. GitHub publication, '
                    'remote CI, team review, and release freeze are separate pending steps. '
                    'Current demonstrations do not establish historical Sprint-1 recording dates.',styles['BodyCustom']),PageBreak(),
          Paragraph('Package guide',styles['H1Custom'])]
sections=[('1','Software requirements specification','SRS_v2.md'),('2','Software test plan and cases','Test_Plan.md'),
          ('3','Architecture and design','Architecture_Design.md'),('4','Backlog and two-sprint plan','Backlog_Sprints.md'),
          ('5','Execution and benchmark evidence','Test_Report.md'),('6','Baseline deviations','Deviations.md'),
          ('7','Submission and release checklist','Submission_Checklist.md')]
for number,title,filename in sections:
    story.append(Paragraph(f'<b>{number}.</b> {title}',styles['BodyCustom']))
story += [Spacer(1,18),Paragraph('Repository context',styles['H2Custom']),
          Paragraph('Contribution fork: github.com/InterstellarMC/SE-MiniProject-FileCompression. '
                    'Team upstream: github.com/ChigilarNishanth/SE-MiniProject-FileCompression. '
                    'The local branch incorporates upstream dc36070. Submitted source documents remain preserved.',styles['BodyCustom']),
          Paragraph('Verification at a glance',styles['H2Custom']),
          Paragraph('21 automated cases passed in Release and ASan/UBSan. Two 500 MB fixtures restored with matching '
                    'SHA-256. ARM64 compression measured about 48 MB/s for text and 33 MB/s for binary. '
                    'Exact host and acceptance limitations appear in the execution report.',styles['BodyCustom']),
          Paragraph('Companion artifacts',styles['H2Custom']),
          Paragraph('Source code, build/CI configuration, individual Markdown documents, backlog.json, raw logs, '
                    'benchmark_results.json, vector SVG diagrams, and 60/120-second current-product demonstration '
                    'videos accompany this PDF in the project folder.',styles['BodyCustom'])]
for number,title,filename in sections:
    story.append(PageBreak())
    story.append(Paragraph('PART '+number,styles['CoverSmall']))
    story.extend(markdown_flowables(ROOT/'docs'/filename))


def page_decoration(canvas,doc):
    canvas.saveState()
    canvas.setStrokeColor(TEAL); canvas.setLineWidth(1)
    canvas.line(52,45,A4[0]-52,45)
    canvas.setFont('Helvetica',8); canvas.setFillColor(GRAY)
    canvas.drawString(52,31,'Team E17 | czip | Review candidate | 27 October 2026')
    canvas.drawRightString(A4[0]-52,31,str(doc.page))
    if doc.page>1:
        canvas.setFont('Helvetica',8)
        canvas.drawString(52,A4[1]-30,'SOFTWARE ENGINEERING MINI-PROJECT')
        canvas.drawRightString(A4[0]-52,A4[1]-30,'PARTS 1 + 2')
    canvas.restoreState()


document=SimpleDocTemplate(str(OUTPUT/'SE_Mini_Project_Submission.pdf'),pagesize=A4,
                           leftMargin=52,rightMargin=52,topMargin=54,bottomMargin=62,
                           title='Team E17 - File Compression Tool - Deliverables Parts 1 and 2',
                           author='Team E17',subject='Review package prepared 27 October 2026')
document.build(story,onFirstPage=page_decoration,onLaterPages=page_decoration)
print(OUTPUT/'SE_Mini_Project_Submission.pdf')
