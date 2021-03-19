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

SUITE = ["citycar-sat14-adl:p4-2-2-0-1.pddl", "openstacks-sat08-strips:p30.pddl", "hiking-sat14-strips:ptesting-2-2-8.pddl", "openstacks-agl14-strips:p170_2.pddl", "parcprinter-08-strips:p22.pddl", "thoughtful-sat14-strips:target-typed-25.pddl", "transport-opt11-strips:p07.pddl", "organic-synthesis-split-opt18-strips:p15.pddl", "grid:prob01.pddl", "depot:p16.pddl", "trucks-strips:p13.pddl", "blocks:probBLOCKS-5-0.pddl", "thoughtful-mco14-strips:p13_7_72-typed.pddl", "petri-net-alignment-opt18-strips:p01.pddl", "freecell:probfreecell-5-2.pddl", "floortile-opt14-strips:p03-6-4-2.pddl", "gripper:prob07.pddl", "caldera-split-opt18-adl:p06.pddl", "data-network-opt18-strips:p05.pddl", "openstacks-sat11-strips:p20.pddl", "visitall-sat14-strips:pfile59.pddl", "floortile-sat11-strips:seq-p09-018.pddl", "visitall-sat11-strips:problem14.pddl", "tidybot-opt14-strips:p04.pddl", "snake-opt18-strips:p12.pddl", "airport-adl:p22-airport4halfMUC-p3.pddl", "termes-sat18-strips:p02.pddl", "sokoban-sat11-strips:p09.pddl", "maintenance-sat14-adl:maintenance-1-3-100-300-5-000.pddl", "floortile-opt11-strips:opt-p03-005.pddl", "cavediving-14-adl:testing14_easy.pddl", "miconic-fulladl:f21-3.pddl", "termes-opt18-strips:p10.pddl", "barman-opt14-strips:p638-1.pddl", "organic-synthesis-sat18-strips:p03.pddl", "hiking-opt14-strips:ptesting-2-3-4.pddl", "childsnack-sat14-strips:child-snack_pfile08-2.pddl", "miconic-simpleadl:s10-3.pddl", "pipesworld-notankage:p09-net1-b14-g6.pddl", "schedule:probschedule-41-2.pddl", "openstacks-strips:p08.pddl", "parking-opt14-strips:p_18_10-04.pddl", "floortile-sat14-strips:p04-6-5-2.pddl", "transport-sat11-strips:p15.pddl", "woodworking-sat11-strips:p01.pddl", "tidybot-sat11-strips:p04.pddl", "organic-synthesis-split-sat18-strips:p20.pddl", "mprime:prob11.pddl", "flashfill-sat18-adl:p02.pddl", "settlers-opt18-adl:p10.pddl", "citycar-opt14-adl:p3-3-2-2-2.pddl", "childsnack-opt14-strips:child-snack_pfile04-2.pddl", "optical-telegraphs:p02-opt3.pddl", "agricola-sat18-strips:p16.pddl", "scanalyzer-sat11-strips:p20.pddl", "spider-sat18-strips:p20.pddl", "miconic:s20-0.pddl", "openstacks-opt11-strips:p07.pddl", "pegsol-08-strips:p03.pddl", "scanalyzer-opt11-strips:p15.pddl", "visitall-opt11-strips:problem08-half.pddl", "elevators-sat08-strips:p29.pddl", "barman-opt11-strips:pfile03-011.pddl", "nurikabe-sat18-adl:p16.pddl", "scanalyzer-08-strips:p30.pddl", "tidybot-opt11-strips:p07.pddl", "mystery:prob09.pddl", "woodworking-opt11-strips:p13.pddl", "nomystery-opt11-strips:p04.pddl", "parcprinter-opt11-strips:p17.pddl", "logistics00:probLOGISTICS-15-1.pddl", "assembly:prob11.pddl", "openstacks:p25.pddl", "caldera-sat18-adl:p15.pddl", "parking-sat14-strips:p_28_2.pddl", "parcprinter-sat11-strips:p19.pddl", "elevators-opt11-strips:p15.pddl", "parking-opt11-strips:pfile07-027.pddl", "driverlog:p12.pddl", "transport-sat08-strips:p20.pddl", "agricola-opt18-strips:p08.pddl", "data-network-sat18-strips:p10.pddl", "elevators-opt08-strips:p06.pddl", "ged-opt14-strips:d-3-1.pddl", "pipesworld-tankage:p10-net1-b14-g8-t50.pddl", "psr-small:p14-s23-n2-l3-f70.pddl", "satellite:p14-pfile14.pddl", "elevators-sat11-strips:p15.pddl", "barman-sat11-strips:pfile06-023.pddl", "pegsol-opt11-strips:p15.pddl", "barman-sat14-strips:p4-11-5-15.pddl", "organic-synthesis-opt18-strips:p06.pddl", "openstacks-sat14-strips:p210_1.pddl", "pathways:p21.pddl", "rovers:p08.pddl", "woodworking-opt08-strips:p29.pddl", "sokoban-opt08-strips:p05.pddl", "transport-opt08-strips:p01.pddl", "logistics98:prob22.pddl", "hiking-agl14-strips:testing-3-4-7.pddl", "visitall-opt14-strips:p-1-7.pddl", "maintenance-opt14-adl:maintenance-1-3-015-020-2-002.pddl", "barman-mco14-strips:p4-11-5-15.pddl", "spider-opt18-strips:p06.pddl", "tetris-opt14-strips:p03-8.pddl", "trucks:p02.pddl", "caldera-opt18-adl:p14.pddl", "psr-large:p04-s66-n5-l2-f50.pddl", "tpp:p11.pddl", "caldera-split-sat18-adl:p14.pddl", "ged-sat14-strips:d-12-7.pddl", "openstacks-sat08-adl:p01.pddl", "snake-sat18-strips:p13.pddl", "tetris-sat14-strips:p028.pddl", "openstacks-opt08-strips:p17.pddl", "nomystery-sat11-strips:p01.pddl", "philosophers:p36-phil37.pddl", "storage:p07.pddl", "psr-middle:p27-s86-n6-l3-f50.pddl", "pegsol-sat11-strips:p02.pddl", "settlers-sat18-adl:p15.pddl", "sokoban-opt11-strips:p08.pddl", "sokoban-sat08-strips:p03.pddl", "parking-sat11-strips:pfile11-042.pddl", "woodworking-sat08-strips:p09.pddl", "zenotravel:p13.pddl", "transport-opt14-strips:p03.pddl", "openstacks-opt14-strips:p25_1.pddl", "movie:prob25.pddl", "airport:p40-airport5MUC-p4.pddl", "openstacks-opt08-adl:p24.pddl", "nurikabe-opt18-adl:p20.pddl", "transport-sat14-strips:p09.pddl"]
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
