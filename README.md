The repository for the project of group d603f21 of Aalborg University. Our code is built on top of the Fast Downward planner.

The project revolves around op-mutexes in planning tasks and the inference of these.

# Merge and shrink configurations

We used the following Fast Downward search configurations to test our inference algorithms:


For finding the best configuration using the None algorithm:

pcmp-bis: astar(merge_and_shrink(merge_strategy=merge_precomputed(linear()), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(previous_ops=NoPO), stop_early=true))

pcmp-bhh: astar(merge_and_shrink(merge_strategy=merge_precomputed(linear()), shrink_strategy=shrink_fh(), op_mutex=op_mutex(previous_ops=NoPO), stop_early=true))

sccs-bis: astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(previous_ops=NoPO), stop_early=true))

sccs-fh: astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_fh(), op_mutex=op_mutex(previous_ops=NoPO), stop_early=true))

stlss-bis: astar(merge_and_shrink(merge_strategy=merge_stateless(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(previous_ops=NoPO), stop_early=true))

stlss-fh: astar(merge_and_shrink(merge_strategy=merge_stateless(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_fh(), op_mutex=op_mutex(previous_ops=NoPO), stop_early=true))


Inference algorithms:

None: astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(previous_ops=NoPO), stop_early=true))

Naive: astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(previous_ops=nasutpo), stop_early=true))

COLS: astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(previous_ops=nelutpo), stop_early=true))

BDD-A: astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(previous_ops=bddolmpo(approximate=true, approximation_technique=2,variable_ordering=0)), stop_early=true))

BDD: astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(previous_ops=bddolmpo(approximate=false, approximation_technique=2,variable_ordering=0)), stop_early=true))


None algorith with intermediate vs without intermediate:

With intermediate: astar(merge_and_shrink(op_mutex=op_mutex(previous_ops=NoPO, use_intermediate=true), stop_early=true))

Without intermediate: astar(merge_and_shrink(op_mutex=op_mutex(previous_ops=NoPO, use_intermediate=false), stop_early=true))


Approximation algorithms for BDD algorithms:

Heavy-branch: astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(reachability=no_goal, previous_ops=BDDOLMPO(approximate=false, approximation_technique=1,variable_ordering=0)), stop_early=true))

Overapprox: astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(reachability=no_goal, previous_ops=BDDOLMPO(approximate=false, approximation_technique=2,variable_ordering=0)), stop_early=true))

None: astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(reachability=no_goal, previous_ops=BDDOLMPO(approximate=false, approximation_technique=2,variable_ordering=0)), stop_early=true))


Variable ordering for BDD algorithms:

None: astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(reachability=no_goal, previous_ops=BDDOLMPO(approximate=false, approximation_technique=2,variable_ordering=0)), stop_early=true))

Annealing: astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(reachability=no_goal, previous_ops=BDDOLMPO(approximate=false, approximation_technique=2,variable_ordering=1)), stop_early=true))

Genetic: astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(reachability=no_goal, previous_ops=BDDOLMPO(approximate=false, approximation_technique=2,variable_ordering=2)), stop_early=true))

Symm sift: astar(merge_and_shrink(merge_strategy=merge_sccs(merge_selector=score_based_filtering(scoring_functions=[goal_relevance,dfp,total_order(atomic_ts_order=reverse_level,product_ts_order=new_to_old,atomic_before_product=true)])), shrink_strategy=shrink_bisimulation(), op_mutex=op_mutex(reachability=no_goal, previous_ops=BDDOLMPO(approximate=false, approximation_technique=2,variable_ordering=5)), stop_early=true))


Domains we used for test suite:

["agricola-opt18-strips", "barman-opt11-strips", "barman-opt14-strips", "caldera-opt18-adl", "cavediving-14-adl", "childsnack-opt14-strips", "floortile-opt11-strips", "floortile-opt14-strips", "hiking-opt14-strips", "nomystery-opt11-strips", "nurikabe-opt18-adl", "openstacks", "openstacks-opt08-strips", "openstacks-opt11-strips", "openstacks-opt14-strips", "organic-synthesis-opt18-strips", "parcprinter-08-strips", "parcprinter-opt11-strips", "pathways", "pegsol-08-strips", "pegsol-opt11-strips", "petri-net-alignment-opt18-strips", "pipesworld-notankage", "pipesworld-tankage", "rovers", "snake-opt18-strips", "sokoban-opt08-strips", "sokoban-opt11-strips", "spider-opt18-strips", "tidybot-opt11-strips", "tidybot-opt14-strips", "tpp", "trucks", "woodworking-opt08-strips", "woodworking-opt11-strips"]



# Fast Downward

Fast Downward is a domain-independent classical planning system.

Copyright 2003-2020 Fast Downward contributors (see below).

For further information:
- Fast Downward website: <http://www.fast-downward.org>
- Report a bug or file an issue: <http://issues.fast-downward.org>
- Fast Downward mailing list: <https://groups.google.com/forum/#!forum/fast-downward>
- Fast Downward main repository: <https://github.com/aibasel/downward>


## Contributors

The following list includes all people that actively contributed to
Fast Downward, i.e. all people that appear in some commits in Fast
Downward's history (see below for a history on how Fast Downward
emerged) or people that influenced the development of such commits.
Currently, this list is sorted by the last year the person has been
active, and in case of ties, by the earliest year the person started
contributing, and finally by last name.

- 2003-2020 Malte Helmert
- 2008-2016, 2018-2020 Gabriele Roeger
- 2010-2020 Jendrik Seipp
- 2010-2011, 2013-2020 Silvan Sievers
- 2012-2020 Florian Pommerening
- 2013, 2015-2020 Salome Eriksson
- 2016-2020 Cedric Geissmann
- 2017-2020 Guillem Francès
- 2018-2020 Augusto B. Corrêa
- 2018-2020 Patrick Ferber
- 2015-2019 Manuel Heusner
- 2017 Daniel Killenberger
- 2016 Yusra Alkhazraji
- 2016 Martin Wehrle
- 2014-2015 Patrick von Reth
- 2015 Thomas Keller
- 2009-2014 Erez Karpas
- 2014 Robert P. Goldman
- 2010-2012 Andrew Coles
- 2010, 2012 Patrik Haslum
- 2003-2011 Silvia Richter
- 2009-2011 Emil Keyder
- 2010-2011 Moritz Gronbach
- 2010-2011 Manuela Ortlieb
- 2011 Vidal Alcázar Saiz
- 2011 Michael Katz
- 2011 Raz Nissim
- 2010 Moritz Goebelbecker
- 2007-2009 Matthias Westphal
- 2009 Christian Muise


## History

The current version of Fast Downward is the merger of three different
projects:

- the original version of Fast Downward developed by Malte Helmert
  and Silvia Richter
- LAMA, developed by Silvia Richter and Matthias Westphal based on
  the original Fast Downward
- FD-Tech, a modified version of Fast Downward developed by Erez
  Karpas and Michael Katz based on the original code

In addition to these three main sources, the codebase incorporates
code and features from numerous branches of the Fast Downward codebase
developed for various research papers. The main contributors to these
branches are Malte Helmert, Gabi Röger and Silvia Richter.


## License

The following directory is not part of Fast Downward as covered by
this license:

- ./src/search/ext

For the rest, the following license applies:

```
Fast Downward is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

Fast Downward is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
```