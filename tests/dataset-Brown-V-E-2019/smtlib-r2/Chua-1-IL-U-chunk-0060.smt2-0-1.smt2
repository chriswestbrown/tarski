(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(declare-fun skoX () Real)
(assert (<  (+ (+ (+ 340820312500000000000000000000 (* skoC 229492187500000000000000000000)) (* skoS 41015625000000000000000000000)) (* skoX (+ (- 81796875000000000000000000) (* skoX (+ 9815625000000000000000 (* skoX (+ (- 785250000000000000) (* skoX (+ 41225625000000 (* skoX (+ (- 1413450000) (* skoX 28269)))))))))))) 0))
(check-sat)