(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoM () Real)
(declare-fun skoCOSS () Real)
(declare-fun skoSINS () Real)
(declare-fun skoS () Real)
(assert (and (not (=  skoM 0)) (=  (+ (+ (- 1) (* skoCOSS skoCOSS)) (* skoSINS skoSINS)) 0) (=  (+ (- 2) skoS) 0) (=  (+ (- 2) skoM) 0)))
(check-sat)