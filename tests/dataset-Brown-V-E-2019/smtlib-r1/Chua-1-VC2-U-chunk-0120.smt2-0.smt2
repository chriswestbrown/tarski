(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(declare-fun skoX () Real)
(assert (and (<=  (+ (+ (+ 19346679687500000000000000000000 (* skoC 1787109375000000000000000000000)) (* skoS (- 7744140625000000000000000000000))) (* skoX (+ (- 4643203125000000000000000000) (* skoX (+ 557184375000000000000000 (* skoX (+ (- 44574750000000000000) (* skoX (+ 2340174375000000 (* skoX (+ (- 80234550000) (* skoX 1604691)))))))))))) 0) (<= (* (- 1) (+ (- 31250000000000) (* skoX (+ 3750000000 (* skoX (+ (- 225000) (* skoX 9)))))) (+ (- 31250000000000) (* skoX (+ 3750000000 (* skoX (+ (- 225000) (* skoX 9))))))) 0) (<= (* (+ (- 31250000000000) (* skoX (+ 3750000000 (* skoX (+ (- 225000) (* skoX 9)))))) (+ (- 31250000000000) (* skoX (+ 3750000000 (* skoX (+ (- 225000) (* skoX 9))))))) 0) (<= (* (- 1) skoX) 0) (<=  skoX 0) (<= (* (- 1) (+ (* skoC (- 3)) (* skoS 13))) 0) (>  (+ (* skoC (- 3)) (* skoS 13)) 0) (=  (+ (+ (- 1) (* skoC skoC)) (* skoS skoS)) 0) (<=  (+ (- 289) skoX) 0)))
(check-sat)