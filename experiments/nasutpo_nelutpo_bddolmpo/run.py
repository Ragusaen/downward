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
    (f"{index:02d}-{h_nick}", ["--search", f"astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(previous_ops={o}), stop_early=true))"])
    for index, (h_nick, o) in enumerate(
        [
            ("nasutpo", "nasutpo"),
            ("nelutpo", "nelutpo"),
            ("bddolmpo_t", "bddolmpo(approximate=true)"),
            ("bddolmpo_f", "bddolmpo(approximate=false)"),
        ],
        start=1,
    )
]

BUILD_OPTIONS = []
DRIVER_OPTIONS = ['--validate', '--overall-time-limit', '5m', '--overall-memory-limit', '9000M']

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

#Run on entire strips suite
SUITE = ["agricola-opt18-strips", "barman-opt11-strips", "barman-opt14-strips", "caldera-opt18-adl", "cavediving-14-adl", "childsnack-opt14-strips", "floortile-opt11-strips", "floortile-opt14-strips", "hiking-opt14-strips", "nomystery-opt11-strips", "nurikabe-opt18-adl", "openstacks", "openstacks-opt08-strips", "openstacks-opt11-strips", "openstacks-opt14-strips", "organic-synthesis-opt18-strips", "parcprinter-08-strips", "parcprinter-opt11-strips", "pathways", "pegsol-08-strips", "pegsol-opt11-strips", "petri-net-alignment-opt18-strips", "pipesworld-notankage", "pipesworld-tankage", "rovers", "snake-opt18-strips", "sokoban-opt08-strips", "sokoban-opt11-strips", "spider-opt18-strips", "tidybot-opt11-strips", "tidybot-opt14-strips", "tpp", "trucks", "woodworking-opt08-strips", "woodworking-opt11-strips"]

#exp.add_suite("../../../benchmarks", SUITE)
exp.add_suite("../../../benchmarks", ["freecell:p01.pddl", "freecell:p02.pddl", "freecell:p03.pddl"])


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
    project.OPMUTEX,
]
report = AbsoluteReport(attributes=ATTRIBUTES)
report2 = AbsoluteReport(attributes=ATTRIBUTES, format="tex")

name = "test-abs"
outfile = f"{name}.{report.output_format}"

exp.add_report(report, name=name, outfile=outfile)
exp.add_report(report2, name=name, outfile=outfile)

exp.add_parse_again_step()

exp.run_steps()
