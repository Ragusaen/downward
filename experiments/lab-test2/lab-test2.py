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
REV = "main"

CONFIGS = [
    (f"{index:02d}-{h_nick}", ["--search", f"astar(merge_and_shrink(op_mutex=op_mutex({h})))"])
    for index, (h_nick, h) in enumerate(
        [
            ("NaSUTPO_g", "use_previous_ops=NaSUTPO, reachability_strategy=goal"),
            ("NoPO_g", "use_previous_ops=NoPO, reachability_strategy=goal"),
        ],
        start=1,
    )
]

BUILD_OPTIONS = []
DRIVER_OPTIONS = ['--validate', '--overall-time-limit', '10m', '--overall-memory-limit', '6500M']

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

SUITE = ["agricola-opt18-strips:p01.pddl", "agricola-opt18-strips:p02.pddl", "agricola-opt18-strips:p03.pddl", "airport:p01-airport1-p1.pddl", "airport:p02-airport1-p1.pddl", "airport:p03-airport1-p2.pddl", "barman-opt11-strips:pfile01-001.pddl", "barman-opt11-strips:pfile01-002.pddl", "barman-opt11-strips:pfile01-003.pddl", "barman-opt14-strips:p435-1.pddl", "barman-opt14-strips:p435-2.pddl", "barman-opt14-strips:p435-3.pddl", "blocks:probBLOCKS-10-0.pddl", "blocks:probBLOCKS-10-1.pddl", "blocks:probBLOCKS-10-2.pddl", "childsnack-opt14-strips:child-snack_pfile01-2.pddl", "childsnack-opt14-strips:child-snack_pfile01.pddl", "childsnack-opt14-strips:child-snack_pfile02-2.pddl", "data-network-opt18-strips:p01.pddl", "data-network-opt18-strips:p02.pddl", "data-network-opt18-strips:p03.pddl", "depot:p01.pddl", "depot:p02.pddl", "depot:p03.pddl", "driverlog:p01.pddl", "driverlog:p02.pddl", "driverlog:p03.pddl", "elevators-opt08-strips:p01.pddl", "elevators-opt08-strips:p02.pddl", "elevators-opt08-strips:p03.pddl", "elevators-opt11-strips:p01.pddl", "elevators-opt11-strips:p02.pddl", "elevators-opt11-strips:p03.pddl", "floortile-opt11-strips:opt-p01-001.pddl", "floortile-opt11-strips:opt-p01-002.pddl", "floortile-opt11-strips:opt-p02-003.pddl", "floortile-opt14-strips:p01-4-3-2.pddl", "floortile-opt14-strips:p01-4-4-2.pddl", "floortile-opt14-strips:p01-5-3-2.pddl", "freecell:p01.pddl", "freecell:p02.pddl", "freecell:p03.pddl", "ged-opt14-strips:d-1-2.pddl", "ged-opt14-strips:d-1-3.pddl", "ged-opt14-strips:d-1-4.pddl", "grid:prob01.pddl", "grid:prob02.pddl", "grid:prob03.pddl", "gripper:prob01.pddl", "gripper:prob02.pddl", "gripper:prob03.pddl", "hiking-opt14-strips:ptesting-1-2-3.pddl", "hiking-opt14-strips:ptesting-1-2-4.pddl", "hiking-opt14-strips:ptesting-1-2-5.pddl", "logistics00:probLOGISTICS-10-0.pddl", "logistics00:probLOGISTICS-10-1.pddl", "logistics00:probLOGISTICS-11-0.pddl", "logistics98:prob01.pddl", "logistics98:prob02.pddl", "logistics98:prob03.pddl", "miconic:s1-0.pddl", "miconic:s1-1.pddl", "miconic:s1-2.pddl", "movie:prob01.pddl", "movie:prob02.pddl", "movie:prob03.pddl", "mprime:prob01.pddl", "mprime:prob02.pddl", "mprime:prob03.pddl", "mystery:prob01.pddl", "mystery:prob02.pddl", "mystery:prob03.pddl", "nomystery-opt11-strips:p01.pddl", "nomystery-opt11-strips:p02.pddl", "nomystery-opt11-strips:p03.pddl", "openstacks-opt08-strips:p01.pddl", "openstacks-opt08-strips:p02.pddl", "openstacks-opt08-strips:p03.pddl", "openstacks-opt11-strips:p01.pddl", "openstacks-opt11-strips:p02.pddl", "openstacks-opt11-strips:p03.pddl", "openstacks-opt14-strips:p20_1.pddl", "openstacks-opt14-strips:p20_2.pddl", "openstacks-opt14-strips:p20_3.pddl", "openstacks-strips:p01.pddl", "openstacks-strips:p02.pddl", "openstacks-strips:p03.pddl", "organic-synthesis-opt18-strips:p01.pddl", "organic-synthesis-opt18-strips:p02.pddl", "organic-synthesis-opt18-strips:p03.pddl", "organic-synthesis-split-opt18-strips:p01.pddl", "organic-synthesis-split-opt18-strips:p02.pddl", "organic-synthesis-split-opt18-strips:p03.pddl", "parcprinter-08-strips:p01.pddl", "parcprinter-08-strips:p02.pddl", "parcprinter-08-strips:p03.pddl", "parcprinter-opt11-strips:p01.pddl", "parcprinter-opt11-strips:p02.pddl", "parcprinter-opt11-strips:p03.pddl", "parking-opt11-strips:pfile03-011.pddl", "parking-opt11-strips:pfile03-012.pddl", "parking-opt11-strips:pfile04-013.pddl", "parking-opt14-strips:p_12_7-01.pddl", "parking-opt14-strips:p_12_7-02.pddl", "parking-opt14-strips:p_12_7-03.pddl", "pathways:p01.pddl", "pathways:p02.pddl", "pathways:p03.pddl", "pegsol-08-strips:p01.pddl", "pegsol-08-strips:p02.pddl", "pegsol-08-strips:p03.pddl", "pegsol-opt11-strips:p01.pddl", "pegsol-opt11-strips:p02.pddl", "pegsol-opt11-strips:p03.pddl", "petri-net-alignment-opt18-strips:p01.pddl", "petri-net-alignment-opt18-strips:p02.pddl", "petri-net-alignment-opt18-strips:p03.pddl", "pipesworld-notankage:p01-net1-b6-g2.pddl", "pipesworld-notankage:p02-net1-b6-g4.pddl", "pipesworld-notankage:p03-net1-b8-g3.pddl", "pipesworld-tankage:p01-net1-b6-g2-t50.pddl", "pipesworld-tankage:p02-net1-b6-g4-t50.pddl", "pipesworld-tankage:p03-net1-b8-g3-t80.pddl", "psr-small:p01-s2-n1-l2-f50.pddl", "psr-small:p02-s5-n1-l3-f30.pddl", "psr-small:p03-s7-n1-l3-f70.pddl", "rovers:p01.pddl", "rovers:p02.pddl", "rovers:p03.pddl", "satellite:p01-pfile1.pddl", "satellite:p02-pfile2.pddl", "satellite:p03-pfile3.pddl", "scanalyzer-08-strips:p01.pddl", "scanalyzer-08-strips:p02.pddl", "scanalyzer-08-strips:p03.pddl", "scanalyzer-opt11-strips:p01.pddl", "scanalyzer-opt11-strips:p02.pddl", "scanalyzer-opt11-strips:p03.pddl", "snake-opt18-strips:p01.pddl", "snake-opt18-strips:p02.pddl", "snake-opt18-strips:p03.pddl", "sokoban-opt08-strips:p01.pddl", "sokoban-opt08-strips:p02.pddl", "sokoban-opt08-strips:p03.pddl", "sokoban-opt11-strips:p01.pddl", "sokoban-opt11-strips:p02.pddl", "sokoban-opt11-strips:p03.pddl", "spider-opt18-strips:p01.pddl", "spider-opt18-strips:p02.pddl", "spider-opt18-strips:p03.pddl", "storage:p01.pddl", "storage:p02.pddl", "storage:p03.pddl", "termes-opt18-strips:p01.pddl", "termes-opt18-strips:p02.pddl", "termes-opt18-strips:p03.pddl", "tetris-opt14-strips:p01-10.pddl", "tetris-opt14-strips:p01-6.pddl", "tetris-opt14-strips:p01-8.pddl", "tidybot-opt11-strips:p01.pddl", "tidybot-opt11-strips:p02.pddl", "tidybot-opt11-strips:p03.pddl", "tidybot-opt14-strips:p01.pddl", "tidybot-opt14-strips:p02.pddl", "tidybot-opt14-strips:p03.pddl", "tpp:p01.pddl", "tpp:p02.pddl", "tpp:p03.pddl", "transport-opt08-strips:p01.pddl", "transport-opt08-strips:p02.pddl", "transport-opt08-strips:p03.pddl", "transport-opt11-strips:p01.pddl", "transport-opt11-strips:p02.pddl", "transport-opt11-strips:p03.pddl", "transport-opt14-strips:p01.pddl", "transport-opt14-strips:p02.pddl", "transport-opt14-strips:p03.pddl", "trucks-strips:p01.pddl", "trucks-strips:p02.pddl", "trucks-strips:p03.pddl", "visitall-opt11-strips:problem02-full.pddl", "visitall-opt11-strips:problem02-half.pddl", "visitall-opt11-strips:problem03-full.pddl", "visitall-opt14-strips:p-05-10.pddl", "visitall-opt14-strips:p-05-5.pddl", "visitall-opt14-strips:p-05-6.pddl", "woodworking-opt08-strips:p01.pddl", "woodworking-opt08-strips:p02.pddl", "woodworking-opt08-strips:p03.pddl", "woodworking-opt11-strips:p01.pddl", "woodworking-opt11-strips:p02.pddl", "woodworking-opt11-strips:p03.pddl", "zenotravel:p01.pddl", "zenotravel:p02.pddl", "zenotravel:p03.pddl", 
]

exp.add_suite("../../../benchmarks", "gripper:prob01.pddl")


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
    "num_labels",
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
exp.add_parse_again_step()

exp.run_steps()
