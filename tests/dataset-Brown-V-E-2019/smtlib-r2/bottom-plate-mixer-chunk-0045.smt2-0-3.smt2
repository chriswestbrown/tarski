(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(declare-fun skoCB () Real)
(declare-fun skoSB () Real)
(declare-fun skoX () Real)
(assert (and (<  (+ (+ (- 760000) (* skoC 3400)) (* skoS 7383)) 0) (=  (+ (+ (- 1) (* skoCB skoCB)) (* skoSB skoSB)) 0) (=  (+ (+ (- 1) (* skoC skoC)) (* skoS skoS)) 0) (=  (+ (- 1) (* skoX 10000000)) 0) (>  skoX 0)))
(check-sat)