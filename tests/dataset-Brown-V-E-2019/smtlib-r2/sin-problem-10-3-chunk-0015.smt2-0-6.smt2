(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoCM1 () Real)
(declare-fun skoX () Real)
(declare-fun skoC () Real)
(declare-fun skoCP1 () Real)
(assert (and (not (=  (+ (+ (- 1) (* skoCM1 (* skoCM1 (* skoCM1 (- 1))))) skoX) 0)) (=  (+ (* skoX (- 1)) (* skoC (* skoC skoC))) 0) (>  skoCP1 0) (>  skoCM1 0) (=  skoC 0) (>  (+ (- 7) (* skoX 5)) 0)))
(check-sat)
