(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoCM1 () Real)
(declare-fun skoCP1 () Real)
(assert (<  (+ (+ (- 10) (* skoCM1 (+ 120 (* skoCM1 (+ (- 600) (* skoCM1 1200)))))) (* skoCP1 (+ (- 1) (* skoCM1 (+ 252 (* skoCM1 (+ (- 60) (* skoCM1 2520)))))))) 0))
(check-sat)