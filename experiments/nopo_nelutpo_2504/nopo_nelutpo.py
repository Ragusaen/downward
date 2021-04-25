#! /usr/bin/env python

import os

from pathlib import Path

import project

from lab import tools
from lab.environments import LocalEnvironment
from downward.experiment import FastDownwardExperiment
from downward.reports.absolute import AbsoluteReport
from downward.reports.scatter import ScatterPlotReport


env = LocalEnvironment(processes=4)
exp = FastDownwardExperiment(environment=env)

# Build

exp.add_step("build", exp.build)

# Run

exp.add_step("start", exp.start_runs)

# Search algorithms

REPO = project.get_repo_base()
REV = "lab-test"

CONFIGS = [
    (f"{index:02d}-{h_nick}", ["--search", f"astar(merge_and_shrink(op_mutex=op_mutex({h}), stop_early=true))"])
    for index, (h_nick, h) in enumerate(
        [
            ("NeLUTPO", "use_previous_ops=NeLUTPO, reachability_strategy=goal"),
            ("NoPO", "use_previous_ops=NoPO, reachability_strategy=goal"),
        ],
        start=1,
    )
]

BUILD_OPTIONS = []
DRIVER_OPTIONS = ['--validate', '--overall-time-limit', '20m', '--overall-memory-limit', '9000M']

for config_nick, config in CONFIGS:
    exp.add_algorithm(
        config_nick,
        REPO,
        REV,
        config,
        build_options=BUILD_OPTIONS,
        driver_options=DRIVER_OPTIONS,
    )

#Domains and problems. must use environment variables or have benchmarks and downward REPO in same folder

SUITE = ["agricola-opt18-strips:p05.pddl", "agricola-opt18-strips:p06.pddl", "agricola-opt18-strips:p07.pddl", "agricola-opt18-strips:p08.pddl", "agricola-opt18-strips:p09.pddl", "agricola-opt18-strips:p10.pddl", "airport:p05-airport2-p1.pddl", "airport:p06-airport2-p2.pddl", "airport:p07-airport2-p2.pddl", "airport:p08-airport2-p3.pddl", "airport:p09-airport2-p4.pddl", "airport:p10-airport3-p1.pddl", "barman-opt11-strips:pfile02-005.pddl", "barman-opt11-strips:pfile02-006.pddl", "barman-opt11-strips:pfile02-007.pddl", "barman-opt11-strips:pfile02-008.pddl", "barman-opt11-strips:pfile03-009.pddl", "barman-opt11-strips:pfile03-010.pddl", "barman-opt14-strips:p536-2.pddl", "barman-opt14-strips:p536-3.pddl", "barman-opt14-strips:p638-1.pddl", "barman-opt14-strips:p638-2.pddl", "barman-opt14-strips:p638-3.pddl", "barman-opt14-strips:p739-1.pddl", "blocks:probBLOCKS-11-1.pddl", "blocks:probBLOCKS-11-2.pddl", "blocks:probBLOCKS-12-0.pddl", "blocks:probBLOCKS-12-1.pddl", "blocks:probBLOCKS-13-0.pddl", "blocks:probBLOCKS-13-1.pddl", "childsnack-opt14-strips:child-snack_pfile03-2.pddl", "childsnack-opt14-strips:child-snack_pfile03.pddl", "childsnack-opt14-strips:child-snack_pfile04-2.pddl", "childsnack-opt14-strips:child-snack_pfile04.pddl", "childsnack-opt14-strips:child-snack_pfile05-2.pddl", "childsnack-opt14-strips:child-snack_pfile05.pddl", "data-network-opt18-strips:p05.pddl", "data-network-opt18-strips:p06.pddl", "data-network-opt18-strips:p07.pddl", "data-network-opt18-strips:p08.pddl", "data-network-opt18-strips:p09.pddl", "data-network-opt18-strips:p10.pddl", "depot:p05.pddl", "depot:p06.pddl", "depot:p07.pddl", "depot:p08.pddl", "depot:p09.pddl", "depot:p10.pddl", "driverlog:p05.pddl", "driverlog:p06.pddl", "driverlog:p07.pddl", "driverlog:p08.pddl", "driverlog:p09.pddl", "driverlog:p10.pddl", "elevators-opt08-strips:p05.pddl", "elevators-opt08-strips:p06.pddl", "elevators-opt08-strips:p07.pddl", "elevators-opt08-strips:p08.pddl", "elevators-opt08-strips:p09.pddl", "elevators-opt08-strips:p10.pddl", "elevators-opt11-strips:p05.pddl", "elevators-opt11-strips:p06.pddl", "elevators-opt11-strips:p07.pddl", "elevators-opt11-strips:p08.pddl", "elevators-opt11-strips:p09.pddl", "elevators-opt11-strips:p10.pddl", "floortile-opt11-strips:opt-p03-005.pddl", "floortile-opt11-strips:opt-p03-006.pddl", "floortile-opt11-strips:opt-p04-007.pddl", "floortile-opt11-strips:opt-p04-008.pddl", "floortile-opt11-strips:opt-p05-009.pddl", "floortile-opt11-strips:opt-p05-010.pddl", "floortile-opt14-strips:p01-5-5-2.pddl", "floortile-opt14-strips:p01-6-4-2.pddl", "floortile-opt14-strips:p01-6-5-2.pddl", "floortile-opt14-strips:p02-4-4-2.pddl", "floortile-opt14-strips:p02-5-3-2.pddl", "floortile-opt14-strips:p02-5-4-2.pddl", "freecell:p05.pddl", "freecell:p06.pddl", "freecell:p07.pddl", "freecell:p08.pddl", "freecell:p09.pddl", "freecell:p10.pddl", "ged-opt14-strips:d-2-1.pddl", "ged-opt14-strips:d-2-3.pddl", "ged-opt14-strips:d-2-4.pddl", "ged-opt14-strips:d-2-8.pddl", "ged-opt14-strips:d-3-1.pddl", "ged-opt14-strips:d-3-2.pddl", "grid:prob05.pddl", "gripper:prob05.pddl", "gripper:prob06.pddl", "gripper:prob07.pddl", "gripper:prob08.pddl", "gripper:prob09.pddl", "gripper:prob10.pddl", "hiking-opt14-strips:ptesting-1-2-8.pddl", "hiking-opt14-strips:ptesting-2-2-3.pddl", "hiking-opt14-strips:ptesting-2-2-4.pddl", "hiking-opt14-strips:ptesting-2-2-5.pddl", "hiking-opt14-strips:ptesting-2-2-6.pddl", "hiking-opt14-strips:ptesting-2-2-7.pddl", "logistics00:probLOGISTICS-12-0.pddl", "logistics00:probLOGISTICS-12-1.pddl", "logistics00:probLOGISTICS-13-0.pddl", "logistics00:probLOGISTICS-13-1.pddl", "logistics00:probLOGISTICS-14-0.pddl", "logistics00:probLOGISTICS-14-1.pddl", "logistics98:prob05.pddl", "logistics98:prob06.pddl", "logistics98:prob07.pddl", "logistics98:prob08.pddl", "logistics98:prob09.pddl", "logistics98:prob10.pddl", "miconic:s1-4.pddl", "miconic:s10-0.pddl", "miconic:s10-1.pddl", "miconic:s10-2.pddl", "miconic:s10-3.pddl", "miconic:s10-4.pddl", "movie:prob05.pddl", "movie:prob06.pddl", "movie:prob07.pddl", "movie:prob08.pddl", "movie:prob09.pddl", "movie:prob10.pddl", "mprime:prob05.pddl", "mprime:prob06.pddl", "mprime:prob07.pddl", "mprime:prob08.pddl", "mprime:prob09.pddl", "mprime:prob10.pddl", "mystery:prob05.pddl", "mystery:prob06.pddl", "mystery:prob07.pddl", "mystery:prob08.pddl", "mystery:prob09.pddl", "mystery:prob10.pddl", "nomystery-opt11-strips:p05.pddl", "nomystery-opt11-strips:p06.pddl", "nomystery-opt11-strips:p07.pddl", "nomystery-opt11-strips:p08.pddl", "nomystery-opt11-strips:p09.pddl", "nomystery-opt11-strips:p10.pddl", "openstacks-opt08-strips:p05.pddl", "openstacks-opt08-strips:p06.pddl", "openstacks-opt08-strips:p07.pddl", "openstacks-opt08-strips:p08.pddl", "openstacks-opt08-strips:p09.pddl", "openstacks-opt08-strips:p10.pddl", "openstacks-opt11-strips:p05.pddl", "openstacks-opt11-strips:p06.pddl", "openstacks-opt11-strips:p07.pddl", "openstacks-opt11-strips:p08.pddl", "openstacks-opt11-strips:p09.pddl", "openstacks-opt11-strips:p10.pddl", "openstacks-opt14-strips:p25_3.pddl", "openstacks-opt14-strips:p30_1.pddl", "openstacks-opt14-strips:p30_2.pddl", "openstacks-opt14-strips:p30_3.pddl", "openstacks-opt14-strips:p35_1.pddl", "openstacks-opt14-strips:p35_2.pddl", "openstacks-strips:p05.pddl", "openstacks-strips:p06.pddl", "openstacks-strips:p07.pddl", "openstacks-strips:p08.pddl", "openstacks-strips:p09.pddl", "openstacks-strips:p10.pddl", "organic-synthesis-opt18-strips:p05.pddl", "organic-synthesis-opt18-strips:p06.pddl", "organic-synthesis-opt18-strips:p07.pddl", "organic-synthesis-opt18-strips:p08.pddl", "organic-synthesis-opt18-strips:p09.pddl", "organic-synthesis-opt18-strips:p10.pddl", "organic-synthesis-split-opt18-strips:p05.pddl", "organic-synthesis-split-opt18-strips:p06.pddl", "organic-synthesis-split-opt18-strips:p07.pddl", "organic-synthesis-split-opt18-strips:p08.pddl", "organic-synthesis-split-opt18-strips:p09.pddl", "organic-synthesis-split-opt18-strips:p10.pddl", "parcprinter-08-strips:p05.pddl", "parcprinter-08-strips:p06.pddl", "parcprinter-08-strips:p07.pddl", "parcprinter-08-strips:p08.pddl", "parcprinter-08-strips:p09.pddl", "parcprinter-08-strips:p10.pddl", "parcprinter-opt11-strips:p05.pddl", "parcprinter-opt11-strips:p06.pddl", "parcprinter-opt11-strips:p07.pddl", "parcprinter-opt11-strips:p08.pddl", "parcprinter-opt11-strips:p09.pddl", "parcprinter-opt11-strips:p10.pddl", "parking-opt11-strips:pfile04-015.pddl", "parking-opt11-strips:pfile04-016.pddl", "parking-opt11-strips:pfile05-017.pddl", "parking-opt11-strips:pfile05-018.pddl", "parking-opt11-strips:pfile05-019.pddl", "parking-opt11-strips:pfile05-020.pddl", "parking-opt14-strips:p_14_8-01.pddl", "parking-opt14-strips:p_14_8-02.pddl", "parking-opt14-strips:p_14_8-03.pddl", "parking-opt14-strips:p_14_8-04.pddl", "parking-opt14-strips:p_16_9-01.pddl", "parking-opt14-strips:p_16_9-02.pddl", "pathways:p05.pddl", "pathways:p06.pddl", "pathways:p07.pddl", "pathways:p08.pddl", "pathways:p09.pddl", "pathways:p10.pddl", "pegsol-08-strips:p05.pddl", "pegsol-08-strips:p06.pddl", "pegsol-08-strips:p07.pddl", "pegsol-08-strips:p08.pddl", "pegsol-08-strips:p09.pddl", "pegsol-08-strips:p10.pddl", "pegsol-opt11-strips:p05.pddl", "pegsol-opt11-strips:p06.pddl", "pegsol-opt11-strips:p07.pddl", "pegsol-opt11-strips:p08.pddl", "pegsol-opt11-strips:p09.pddl", "pegsol-opt11-strips:p10.pddl", "petri-net-alignment-opt18-strips:p05.pddl", "petri-net-alignment-opt18-strips:p06.pddl", "petri-net-alignment-opt18-strips:p07.pddl", "petri-net-alignment-opt18-strips:p08.pddl", "petri-net-alignment-opt18-strips:p09.pddl", "petri-net-alignment-opt18-strips:p10.pddl", "pipesworld-notankage:p05-net1-b10-g4.pddl", "pipesworld-notankage:p06-net1-b10-g6.pddl", "pipesworld-notankage:p07-net1-b12-g5.pddl", "pipesworld-notankage:p08-net1-b12-g7.pddl", "pipesworld-notankage:p09-net1-b14-g6.pddl", "pipesworld-notankage:p10-net1-b14-g8.pddl", "pipesworld-tankage:p05-net1-b10-g4-t50.pddl", "pipesworld-tankage:p06-net1-b10-g6-t50.pddl", "pipesworld-tankage:p07-net1-b12-g5-t80.pddl", "pipesworld-tankage:p08-net1-b12-g7-t80.pddl", "pipesworld-tankage:p09-net1-b14-g6-t50.pddl", "pipesworld-tankage:p10-net1-b14-g8-t50.pddl", "psr-small:p05-s9-n1-l4-f30.pddl", "psr-small:p06-s10-n1-l4-f50.pddl", "psr-small:p07-s11-n1-l4-f70.pddl", "psr-small:p08-s12-n1-l5-f10.pddl", "psr-small:p09-s13-n1-l5-f30.pddl", "psr-small:p10-s17-n2-l2-f30.pddl", "rovers:p05.pddl", "rovers:p06.pddl", "rovers:p07.pddl", "rovers:p08.pddl", "rovers:p09.pddl", "rovers:p10.pddl", "satellite:p05-pfile5.pddl", "satellite:p06-pfile6.pddl", "satellite:p07-pfile7.pddl", "satellite:p08-pfile8.pddl", "satellite:p09-pfile9.pddl", "satellite:p10-pfile10.pddl", "scanalyzer-08-strips:p05.pddl", "scanalyzer-08-strips:p06.pddl", "scanalyzer-08-strips:p07.pddl", "scanalyzer-08-strips:p08.pddl", "scanalyzer-08-strips:p09.pddl", "scanalyzer-08-strips:p10.pddl", "scanalyzer-opt11-strips:p05.pddl", "scanalyzer-opt11-strips:p06.pddl", "scanalyzer-opt11-strips:p07.pddl", "scanalyzer-opt11-strips:p08.pddl", "scanalyzer-opt11-strips:p09.pddl", "scanalyzer-opt11-strips:p10.pddl", "snake-opt18-strips:p05.pddl", "snake-opt18-strips:p06.pddl", "snake-opt18-strips:p07.pddl", "snake-opt18-strips:p08.pddl", "snake-opt18-strips:p09.pddl", "snake-opt18-strips:p10.pddl", "sokoban-opt08-strips:p05.pddl", "sokoban-opt08-strips:p06.pddl", "sokoban-opt08-strips:p07.pddl", "sokoban-opt08-strips:p08.pddl", "sokoban-opt08-strips:p09.pddl", "sokoban-opt08-strips:p10.pddl", "sokoban-opt11-strips:p05.pddl", "sokoban-opt11-strips:p06.pddl", "sokoban-opt11-strips:p07.pddl", "sokoban-opt11-strips:p08.pddl", "sokoban-opt11-strips:p09.pddl", "sokoban-opt11-strips:p10.pddl", "spider-opt18-strips:p05.pddl", "spider-opt18-strips:p06.pddl", "spider-opt18-strips:p07.pddl", "spider-opt18-strips:p08.pddl", "spider-opt18-strips:p09.pddl", "spider-opt18-strips:p10.pddl", "storage:p05.pddl", "storage:p06.pddl", "storage:p07.pddl", "storage:p08.pddl", "storage:p09.pddl", "storage:p10.pddl", "termes-opt18-strips:p05.pddl", "termes-opt18-strips:p06.pddl", "termes-opt18-strips:p07.pddl", "termes-opt18-strips:p08.pddl", "termes-opt18-strips:p09.pddl", "termes-opt18-strips:p10.pddl", "tetris-opt14-strips:p02-4.pddl", "tetris-opt14-strips:p02-6.pddl", "tetris-opt14-strips:p02-8.pddl", "tetris-opt14-strips:p03-10.pddl", "tetris-opt14-strips:p03-4.pddl", "tetris-opt14-strips:p03-6.pddl", "tidybot-opt11-strips:p05.pddl", "tidybot-opt11-strips:p06.pddl", "tidybot-opt11-strips:p07.pddl", "tidybot-opt11-strips:p08.pddl", "tidybot-opt11-strips:p09.pddl", "tidybot-opt11-strips:p10.pddl", "tidybot-opt14-strips:p05.pddl", "tidybot-opt14-strips:p06.pddl", "tidybot-opt14-strips:p07.pddl", "tidybot-opt14-strips:p08.pddl", "tidybot-opt14-strips:p09.pddl", "tidybot-opt14-strips:p10.pddl", "tpp:p05.pddl", "tpp:p06.pddl", "tpp:p07.pddl", "tpp:p08.pddl", "tpp:p09.pddl", "tpp:p10.pddl", "transport-opt08-strips:p05.pddl", "transport-opt08-strips:p06.pddl", "transport-opt08-strips:p07.pddl", "transport-opt08-strips:p08.pddl", "transport-opt08-strips:p09.pddl", "transport-opt08-strips:p10.pddl", "transport-opt11-strips:p05.pddl", "transport-opt11-strips:p06.pddl", "transport-opt11-strips:p07.pddl", "transport-opt11-strips:p08.pddl", "transport-opt11-strips:p09.pddl", "transport-opt11-strips:p10.pddl", "transport-opt14-strips:p05.pddl", "transport-opt14-strips:p06.pddl", "transport-opt14-strips:p07.pddl", "transport-opt14-strips:p08.pddl", "transport-opt14-strips:p09.pddl", "transport-opt14-strips:p10.pddl", "trucks-strips:p05.pddl", "trucks-strips:p06.pddl", "trucks-strips:p07.pddl", "trucks-strips:p08.pddl", "trucks-strips:p09.pddl", "trucks-strips:p10.pddl", "visitall-opt11-strips:problem04-full.pddl", "visitall-opt11-strips:problem04-half.pddl", "visitall-opt11-strips:problem05-full.pddl", "visitall-opt11-strips:problem05-half.pddl", "visitall-opt11-strips:problem06-full.pddl", "visitall-opt11-strips:problem06-half.pddl", "visitall-opt14-strips:p-05-8.pddl", "visitall-opt14-strips:p-05-9.pddl", "visitall-opt14-strips:p-1-10.pddl", "visitall-opt14-strips:p-1-11.pddl", "visitall-opt14-strips:p-1-12.pddl", "visitall-opt14-strips:p-1-13.pddl", "woodworking-opt08-strips:p05.pddl", "woodworking-opt08-strips:p06.pddl", "woodworking-opt08-strips:p07.pddl", "woodworking-opt08-strips:p08.pddl", "woodworking-opt08-strips:p09.pddl", "woodworking-opt08-strips:p10.pddl", "woodworking-opt11-strips", "zenotravel:p05.pddl", "zenotravel:p06.pddl", "zenotravel:p07.pddl", "zenotravel:p08.pddl", "zenotravel:p09.pddl", "zenotravel:p10.pddl", 
]

exp.add_suite("../../../benchmarks", SUITE)
#exp.add_suite("../../../benchmarks", ["freecell:p01.pddl"])


# If using environment variables

# BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]
# exp.add_suite(BENCHMARKS_DIR, SUITE)



# Output fetching

exp.add_fetcher(name="fetch")

# Parsers for fetching

DIR = Path(__file__).resolve().parent

exp.add_parser(exp.EXITCODE_PARSER)
exp.add_parser(exp.TRANSLATOR_PARSER)
exp.add_parser(exp.SINGLE_SEARCH_PARSER)
exp.add_parser(exp.PLANNER_PARSER)
exp.add_parser(DIR / "parser.py")

# Absolute HTML Report

ATTRIBUTES = [
    "operator_mutex_time",
    "operator_mutexes_num",
    #project.EVALUATIONS_PER_TIME,
]

report = AbsoluteReport(attributes=ATTRIBUTES)

name = "test-abs"
outfile = f"{name}.{report.output_format}"

exp.add_report(report, name=name, outfile=outfile)

# ScatterPlot Report

# report = ScatterPlotReport(attributes = "foo")

# suffix = "-rel" if project.RELATIVE else ""
# for algo1, algo2 in pairs:
#     for attr in attributes:
#         exp.add_report(
#             project.ScatterPlotReport(
#                 relative=project.RELATIVE,
#                 get_category=None if project.TEX else lambda run1, run2: run1["domain"],
#                 attributes=[attr],
#                 filter_algorithm=[algo1, algo2],
#                 filter=[project.add_evaluations_per_time],
#                 format="tex" if project.TEX else "png",
#             ),
#             name=f"{exp.name}-{algo1}-vs-{algo2}-{attr}{suffix}",
#         )

exp.run_steps()
