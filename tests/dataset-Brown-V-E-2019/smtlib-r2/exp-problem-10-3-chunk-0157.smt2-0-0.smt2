(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoCM1 () Real)
(declare-fun skoCP1 () Real)
(assert (<  (+ (+ 7 (* skoCM1 (+ (- 2) (* skoCM1 84)))) (* skoCP1 (+ (+ (- 2) (* skoCM1 (+ (- 228) (* skoCM1 (- 24))))) (* skoCP1 (+ 84 (* skoCM1 (+ (- 24) (* skoCM1 1008)))))))) 0))
(check-sat)
