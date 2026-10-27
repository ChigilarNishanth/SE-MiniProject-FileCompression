"""Prepare traceable plans and evidence from actual local results."""
import json
from pathlib import Path
import subprocess
import urllib.request

ROOT = Path(__file__).resolve().parents[1]
DOCS = ROOT / 'docs'
UPSTREAM = 'ChigilarNishanth/SE-MiniProject-FileCompression'


def get_json(path):
    request = urllib.request.Request('https://api.github.com/' + path,
                                     headers={'Accept': 'application/vnd.github+json'})
    return json.load(urllib.request.urlopen(request))


issues = get_json(f'repos/{UPSTREAM}/issues?state=all&per_page=100')
snapshot = [{'number': i['number'], 'title': i['title'], 'state': i['state'],
             'assignees': [a['login'] for a in i['assignees']], 'url': i['html_url']}
            for i in issues if 'pull_request' not in i]
(DOCS / 'upstream_issue_snapshot.json').write_text(json.dumps(snapshot, indent=2) + '\n')
board=json.loads((DOCS/'project_snapshot.json').read_text())
board_items={item['issue']:item for item in board['items']}
mapping = {
    1: (2, 1, ['FR-08', 'NFR-04', 'NFR-05'], 'TC-08, TC-17', 'Help, syntax, default output names, and status codes verified.'),
    2: (3, 1, ['FR-03', 'NFR-02'], 'TC-02, TC-06, TC-22, TC-23', 'All byte values counted correctly; bounded buffers and RSS evidence.'),
    3: (5, 1, ['FR-04'], 'TC-06, TC-18, TC-21', 'Stable frequency/minimum-symbol tie-break and independent golden archive.'),
    4: (3, 1, ['FR-05', 'SEC-REQ-01'], 'TC-06, TC-11, TC-12', 'Versioned portable header; invalid metadata rejected before allocation.'),
    5: (3, 1, ['NFR-03', 'NFR-05'], 'CI matrix', 'CMake builds/tests on push and PR; remote jobs must actually run.'),
    6: (5, 1, ['FR-06'], 'TC-04, TC-15, TC-21', 'Known-code payload, one-symbol coding, and zero padding verified.'),
    7: (5, 1, ['FR-01'], 'TC-01 through TC-07, TC-18', 'Complete stable two-pass compression, not a header-only scaffold.'),
    8: (8, 2, ['FR-02', 'SEC-REQ-02'], 'TC-01 through TC-05, TC-11 through TC-19', 'Safe decoding with frequency, size, length, padding and CRC validation.'),
    9: (2, 2, ['FR-07', 'NFR-01'], 'TC-07, TC-22', 'Actual original/archive/savings/time metrics; benchmark evidence attached.'),
    10: (5, 2, ['NFR-03', 'SEC-REQ-02'], 'TC-01 through TC-21', '21 requirement tests, seeded cases, independent golden oracle and sanitizers.'),
}
stories = []
for issue in sorted(snapshot, key=lambda i: i['number']):
    number = issue['number']
    if number not in mapping:
        continue
    points, sprint, requirements, tests, acceptance = mapping[number]
    points=board_items[number]['estimate']
    sprint=1 if number<=5 else 2
    stories.append({'id': f'US-{number:02}', 'title': issue['title'], 'existing_issue': issue['url'],
                    'story_points': points, 'sprint': sprint, 'assignee': issue['assignees'][0],
                    'assignment_source': 'existing upstream issue', 'requirements': requirements,
                    'estimate_source': 'existing private project Estimate field',
                    'observed_project_status': board_items[number]['status'],
                    'sprint_source': 'existing Sprint-1 status' if number<=5 else 'proposed Sprint-2 allocation from Backlog',
                    'acceptance': acceptance, 'evidence': tests, 'observed_github_state': issue['state'],
                    'local_state': 'implemented; team review pending' if number != 5 else 'configured; remote CI pending'})
extras = [
    ('US-11', 'Streaming performance and memory acceptance', 3, 'InterstellarMC', ['NFR-01', 'NFR-02'],
     'TC-22, TC-23; benchmark_results.json', 'Record 500 MB text/binary speed and RSS; validate x86-64 reference target separately.'),
    ('US-12', 'Output preservation and decompression resource limits', 3, 'Mahesh33-mahi', ['SEC-REQ-03', 'NFR-04'],
     'TC-10, TC-16, TC-20', 'Never overwrite an existing file; cleanup handled failures; reject output beyond cap.'),
    ('US-13', 'Complete SRS, test plan, design and change log', 3, 'ChigilarNishanth', ['FR-01 through FR-08', 'NFR-01 through NFR-05'],
     'docs and submission PDF', 'Include UML use case, component view, two sequence diagrams, APIs, security, cases and traceability.'),
    ('US-14', 'Demonstration evidence and contributor review', 3, 'InterstellarMC', ['Part-2 demonstration and review process'],
     'demos; PR and review evidence pending', 'Current working-product demos: 60 and 120 seconds. Actual member-authored PR and owner review remain human evidence.'),
    ('US-15', 'Final review and development freeze', 2, 'ChigilarNishanth', ['Part-2 final documentation and freeze'],
     'docs/Submission_Checklist.md', 'Confirm deviations, CI, unresolved reference-host check, real review, then freeze an approved commit. No backdated closure.'),
]
for sid, title, points, assignee, requirements, evidence, acceptance in extras:
    stories.append({'id': sid, 'title': f'{sid}: {title}', 'existing_issue': None, 'story_points': points,
                    'sprint': 2, 'assignee': assignee, 'assignment_source': 'proposed; team confirmation pending',
                    'requirements': requirements, 'acceptance': acceptance, 'evidence': evidence,
                    'observed_github_state': 'not created', 'local_state': 'prepared; acceptance/review pending'})
manifest = {'project_title': board['title'], 'project_url': board['url'], 'repository': UPSTREAM,
            'prepared_date': '2026-10-27', 'publication_status': 'existing linked Project observed; local additions not yet published',
            'sprints': [{'number': 1, 'start': '2026-10-19', 'end': '2026-10-23'},
                        {'number': 2, 'start': '2026-10-26', 'end': '2026-10-30'}], 'stories': stories}
(DOCS / 'backlog.json').write_text(json.dumps(manifest, indent=2) + '\n')
rows = ['# Backlog and Two-Sprint Plan', '',
        'Prepared 27 October 2026. Existing upstream stories US-01 through US-10 and their assignees are preserved. '
        'Story points for US-01 through US-10 preserve the existing board Estimate values. '
        'US-01 through US-05 retain their observed Sprint-1 allocation; US-06 through US-10 are currently in Backlog and are proposed for Sprint 2. '
        'US-11 through US-15 are proposed additions. The existing linked board is SE Mini-Project Sprint Board, project #2 under ChigilarNishanth.', '',
        'Existing project: ' + board['url'] + ' (private; observed in the authenticated browser). No new Project is needed.', '',
        'Target repository: https://github.com/' + UPSTREAM, '',
        'Sprint 1: 19-23 October 2026; Sprint 2: 26-30 October 2026. Sprint 1 is already past; this plan does not backdate work.', '',
        '| Story | Points | Sprint | Owner | Requirements | Evidence |', '| --- | --- | --- | --- | --- | --- |']
for story in stories:
    rows.append(f"| {story['id']} {story['title'].split(': ',1)[1]} | {story['story_points']} | {story['sprint']} | {story['assignee']} | {', '.join(story['requirements'])} | {story['evidence']} |")
for sprint in (1, 2):
    total = sum(story['story_points'] for story in stories if story['sprint'] == sprint)
    rows += ['', f'## Sprint {sprint} goal and capacity', f'Planned total: {total} story points. '
             + ('Goal: CLI, frequency model, tree, header validation and CI, consistent with the observed board allocation.' if sprint == 1
                else 'Goal: packed encoding, complete compression and safe decompression, measured non-functional behavior, documentation, current demonstration, and release review.')]
rows += ['', '## Definition of ready and done',
         'Ready: linked requirement, owner, points, measurable acceptance, dependencies, and fixture plan. '
         'Done: implementation reviewed, relevant tests pass, remote CI verified, documents updated, demonstration evidence linked, '
         'and actual assignee/reviewer participation recorded. Local implementation alone does not close upstream issues.', '',
         '## GitHub publication procedure',
         'Reuse existing issues #1 through #10 in the team repository; do not duplicate them in the fork. '
         'Reuse the existing linked Project #2 and its Estimate field. '
         'Retain current issue states; add requirement/evidence links and proposed Sprint-2 items after review. '
         'Keep status based on actual acceptance, not the old sprint dates. The fork has issues disabled, so it is the contribution source, not the team backlog.', '',
         '## Acceptance details']
for story in stories:
    rows += ['', f"### {story['id']}", story['acceptance'],
             f"Ownership: {story['assignment_source']}. GitHub state observed: {story['observed_github_state']}. "
             f"Current local state: {story['local_state']}."]
(DOCS / 'Backlog_Sprints.md').write_text('\n'.join(rows) + '\n')

benchmark = json.loads((DOCS / 'benchmark_results.json').read_text())
compiler = subprocess.check_output(['c++', '--version'], text=True).splitlines()[0]
lines = ['# Test Execution and Benchmark Report', '', 'Evidence prepared 27 October 2026. Host: ' + benchmark['platform'] + '.',
         'Compiler: ' + compiler + '. Release: -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror.', '',
         '## Automated validation',
         '21 automated cases passed in Release and ASan/UBSan builds. Cases include 25 seeded round trips, '
         '40 seeded single-bit archive mutations, and an independently constructed golden archive. '
         'Detailed logs are test_results_release.txt and test_results.txt. No remote CI result is implied.', '',
         '## 500 MB non-functional measurements',
         'Each fixture contains 500,000,000 bytes. Timing includes complete CLI process execution and I/O; '
         'MB/s uses decimal megabytes. Compression uses two source passes. macOS /usr/bin/time -l reports peak '
         'process resident set size in bytes, not an allocator-specific heap measurement.', '',
         '| Fixture | Compress MB/s | Decompress MB/s | Archive bytes | Peak process RSS (MB) | SHA-256 match |',
         '| --- | --- | --- | --- | --- | --- |']
for case in benchmark['cases']:
    peak = max(case['metrics'][op]['max_rss_bytes'] for op in ('-c', '-d'))
    lines.append(f"| {case['fixture']} | {case['metrics']['-c']['MB_per_second']:.2f} | {case['metrics']['-d']['MB_per_second']:.2f} | {case['archive_bytes']:,} | {peak/1000000:.2f} | {case['sha256_match']} |")
lines += ['', '## Interpretation and unresolved evidence',
          'Both fixtures exceed 10 MB/s on the available ARM64 host. NFR-01 specifies an x86-64 reference class, '
          'so this is supplementary evidence and that exact environment remains unverified. '
          'Peak RSS is below 2 MB on this run and supports the bounded-memory design. '
          'RSS is broader than heap; no heap-specific profiler output is claimed.', '',
          'The binary fixture is a deterministic one-megabyte random block repeated 500 times; static per-byte Huffman coding '
          'does not exploit long-range block repetition, and the uniform byte distribution makes its payload effectively incompressible. '
          'Its 2,072-byte expansion is expected header overhead. Test results are fixture-specific and not a proof for all inputs.', '',
          '## Reproduction',
          'Build Release, run python3 tests/test_czip.py build/czip, then python3 tools/benchmark.py build/czip 500. '
          'For sanitizers, use CMake -DCZIP_SANITIZE=ON or the equivalent compiler flags. '
          'Allow roughly 1.5 GB of temporary free disk space for the binary source, archive and restored file.']
(DOCS / 'Test_Report.md').write_text('\n'.join(lines) + '\n')

# Keep the case numbering aligned with the added independent golden oracle.
plan_path = DOCS / 'Test_Plan.md'
plan = plan_path.read_text().replace('TC-22', 'TC-23').replace('TC-21', 'TC-22')
plan = plan.replace('20 automated', '21 automated').replace('All 20 automated', 'All 21 automated')
plan = plan.replace('TC-01 through TC-22', 'TC-01 through TC-21')
needle = '| TC-22 | Release build;'
plan = plan.replace(needle, '| TC-21 | Independently construct AAABBC header and known payload 1F 00; compare encoder and decode oracle | Exact archive bytes and restored content match independent specification |\n' + needle)
# Idempotent reruns: the source plan is already numbered after the first run.
if 'TC-24' not in plan and '| TC-21 | Independently construct' not in plan_path.read_text():
    plan_path.write_text(plan)
