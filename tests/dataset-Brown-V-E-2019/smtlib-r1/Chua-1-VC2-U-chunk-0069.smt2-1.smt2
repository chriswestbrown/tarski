(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(assert (and (<= (* (- 1) (+ 625000000 (* skoX (+ (- 75000) (* skoX 3))))) 0) (<=  (+ 625000000 (* skoX (+ (- 75000) (* skoX 3)))) 0) (<= (* (- 1) (+ (+ (+ (- 12381875000000) (* skoX (+ 1485825000 (* skoX (- 59433))))) (* skoC (+ (- 1143750000000) (* skoX (+ (- 137250000) (* skoX (- 5490))))))) (* skoS (+ 4956250000000 (* skoX (+ 594750000 (* skoX 23790))))))) 0) (<= (* (- 1) (+ (* skoC (- 3)) (* skoS 13))) 0) (> (* (- 1) (+ (* skoC (- 3)) (* skoS 13))) 0) (<= (* (- 1) skoX) 0) (<=  skoX 0) (=  (+ (+ (- 1) (* skoC skoC)) (* skoS skoS)) 0) (<=  (+ (- 289) skoX) 0)))
(check-sat)
