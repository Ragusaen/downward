#! /usr/bin/env python

import os

from pathlib import Path

import project

from lab import tools
from lab.environments import LocalEnvironment
from downward.experiment import FastDownwardExperiment
from downward.reports.absolute import AbsoluteReport
from downward.reports.scatter import ScatterPlotReport


env = LocalEnvironment(processes=2)
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
            #("mas", "merge_and_shrink(merge_strategy=merge_stateless(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])),shrink_strategy=shrink_bisimulation(greedy=false),label_reduction=exact(before_shrinking=true,before_merging=false),max_states=50000,threshold_before_merge=1)"),
            ("op-mutex_no-goal_no-prev", "use_previous_ops=false, reachability_strategy=no_goal"),
            ("op-mutex_goal_no-prev", "use_previous_ops=false, reachability_strategy=goal"),
            ("op-mutex_no-goal_prev", "use_previous_ops=true, reachability_strategy=no_goal"),
            ("op-mutex_goal_prev", "use_previous_ops=true, reachability_strategy=goal"),

        ],
        start=1,
    )
]

BUILD_OPTIONS = []
DRIVER_OPTIONS = ['--validate', '--overall-time-limit', '10m', '--overall-memory-limit', '3584M']

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

SUITE = ["agricola-opt18-strips:p06.pddl", "airport:p10-airport3-p1.pddl", "barman-opt11-strips:pfile01-003.pddl", "barman-opt14-strips:p536-3.pddl", "blocks:probBLOCKS-5-0.pddl", "childsnack-opt14-strips:child-snack_pfile03-2.pddl", "data-network-opt18-strips:p07.pddl", "depot:p16.pddl", "driverlog:p13.pddl", "elevators-opt08-strips:p29.pddl", "elevators-opt11-strips:p16.pddl", "floortile-opt11-strips:opt-p09-017.pddl", "floortile-opt14-strips:p03-5-4-2.pddl", "freecell:probfreecell-9-1.pddl", "ged-opt14-strips:d-2-4.pddl", "grid:prob01.pddl", "gripper:prob20.pddl", "hiking-opt14-strips:ptesting-2-3-6.pddl", "logistics00:probLOGISTICS-12-0.pddl", "logistics98:prob30.pddl", "miconic:s4-4.pddl", "movie:prob21.pddl", "mprime:prob28.pddl", "mystery:prob22.pddl", "nomystery-opt11-strips:p07.pddl", "openstacks-opt08-strips:p07.pddl", "openstacks-opt11-strips:p05.pddl", "openstacks-opt14-strips:p45_3.pddl", "openstacks-strips:p10.pddl", "organic-synthesis-opt18-strips:p04.pddl", "organic-synthesis-split-opt18-strips:p17.pddl", "parcprinter-08-strips:p02.pddl", "parcprinter-opt11-strips:p18.pddl", "parking-opt11-strips:pfile07-025.pddl", "parking-opt14-strips:p_16_9-01.pddl", "pathways:p02.pddl", "pegsol-08-strips:p06.pddl", "pegsol-opt11-strips:p13.pddl", "petri-net-alignment-opt18-strips:p11.pddl", "pipesworld-notankage:p23-net3-b14-g3.pddl", "pipesworld-tankage:p34-net4-b16-g6-t60.pddl", "psr-small:p11-s18-n2-l2-f50.pddl", "rovers:p03.pddl", "satellite:p36-HC-pfile16.pddl", "scanalyzer-08-strips:p04.pddl", "scanalyzer-opt11-strips:p12.pddl", "snake-opt18-strips:p18.pddl", "sokoban-opt08-strips:p24.pddl", "sokoban-opt11-strips:p01.pddl", "spider-opt18-strips:p13.pddl", "storage:p03.pddl", "termes-opt18-strips:p15.pddl", "tetris-opt14-strips:p01-8.pddl", "tidybot-opt11-strips:p01.pddl", "tidybot-opt14-strips:p08.pddl", "tpp:p18.pddl", "transport-opt08-strips:p21.pddl", "transport-opt11-strips:p06.pddl", "transport-opt14-strips:p11.pddl", "trucks-strips:p07.pddl", "visitall-opt11-strips:problem10-full.pddl", "visitall-opt14-strips:p-1-18.pddl", "woodworking-opt08-strips:p18.pddl", "woodworking-opt11-strips:p12.pddl", "zenotravel:p13.pddl"]
]
exp.add_suite("../../../benchmarks", SUITE)


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
