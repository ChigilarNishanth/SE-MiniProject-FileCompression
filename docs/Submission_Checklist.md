# Submission and release checklist

Prepared 27 October 2026. This is a review candidate, not a declaration that the team has frozen or submitted a release.

## Part 1 deliverables
| Item | Evidence / status |
| --- | --- |
| Descriptive SRS and measurable FR/NFR | SRS_v2.md; original submission retained |
| At least one UML use-case diagram | Combined PDF use-case diagram with actors, use cases and include relationships |
| Two security objectives and requirements | SRS section 4; three security requirements |
| Test plan sections 3, 4, 5 and security 5.1 | Test_Plan.md |
| Requirements traceability | Test plan matrix, design mapping, backlog.json |
| Component diagram, pattern, security architecture | Architecture_Design.md and rendered component view |
| Two UML sequence diagrams | Compression and decompression views in combined PDF |
| API and error design | Architecture_Design.md sections 7 and 8 |
| At least 7-10 functional/non-functional cases | 21 automated cases plus 2 benchmark cases |

## Part 2 deliverables
| Item | Evidence / status |
| --- | --- |
| GitHub project linked to mini-project repo | Existing private SE Mini-Project Sprint Board #2 confirmed linked to the team repository |
| Backlog derived from FR and NFR | Existing upstream US-01..10 mapped; US-11..15 proposed in backlog.json |
| Points and team assignment | Existing assignees and Estimate values preserved; proposed additional tasks and Sprint-2 allocations need review |
| Two sprints planned | Backlog_Sprints.md uses original 19-23 and 26-30 October dates without backdating completion |
| Working product | Complete C++17 compressor/decoder; local tests and benchmark logs |
| GitHub Actions | Run 40556449207 passed Linux/macOS/Windows Release and Linux sanitizers for code commit fc2db01 |
| Assigned member raises PR; owner reviews | PR #13 is authored by InterstellarMC; other teammates' acceptance and owner review remain pending |
| One-minute Sprint-1 outcome video | 60-second current-product demonstration supplied; historical Sprint-1 recording not available |
| Two-minute Sprint-2 outcome video | 120-second current-product demonstration supplied |
| Upload videos to repo | Both demonstration videos published on the contribution branch in draft PR #13 |
| Document deviations | Deviations.md with explicit review impacts |
| Complete docs and freeze | Review package prepared; team acceptance and final frozen commit pending |

## Final review procedure
1. Review the actual source, diagrams, test logs, and deviations against the submitted baseline.
2. Publish the contribution branch and open the team's required PR. Retain actual author and reviewer identities; do not impersonate teammates or invent old activity.
3. Run the remote CI matrix and inspect its results. Correct failures before calling all platforms verified.
4. Reuse the existing linked GitHub Project #2, preserve its recorded Estimate values, and review proposed Sprint-2 additions using existing issues wherever possible.
5. Record/confirm the evaluator's acceptance of a current demonstration in place of missing historical Sprint-1 evidence. Historical evidence cannot be manufactured.
6. Validate the x86-64 throughput target or record an approved environment deviation; distinguish process RSS from heap profiling.
7. After owner review and all required acceptance, freeze the approved commit and tag the release. No approval or tag is asserted by this package.

## Published review candidate
Draft PR: https://github.com/ChigilarNishanth/SE-MiniProject-FileCompression/pull/13. It is linked to the existing private board as Sprint 2 - In Progress. Existing issue assignments, estimates and completion states are preserved; additional tasks remain proposals in the review package. No team owner approval or merge has been performed.
