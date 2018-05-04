(set-info :status unsat)
(set-logic QF_NRA)
(set-info :source | Simple test example.|)
(set-info :smt-lib-version 2.0)
(declare-fun x () Real)
(declare-fun y () Real)
1234
(assert (< (/ x 3) y x))
(checksat)
