(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(declare-fun skoX () Real)
(assert (and (> (* (- 1) (+ (+ (+ 36000000000000000000000000 (* skoC (- 27360000000000000000000000))) (* skoS 5400000000000000000000000)) (* skoX (+ 208800000000000000000000 (* skoX (+ 605520000000000000000 (* skoX (+ 1170672000000000000 (* skoX (+ 1485290100000000 (* skoX (+ 1230668940000 (* skoX 594823321))))))))))))) 0) (> (* (+ 6000000000000 (* skoX (+ 17400000000 (* skoX (+ 25230000 (* skoX 24389)))))) (+ 6000000000000 (* skoX (+ 17400000000 (* skoX (+ 25230000 (* skoX 24389))))))) 0) (<= (* (- 1) skoX) 0) (<=  skoX 0) (<=  (+ (* skoC (- 76)) (* skoS 15)) 0) (>  (+ (* skoC (- 76)) (* skoS 15)) 0) (=  (+ (+ (- 1) (* skoC skoC)) (* skoS skoS)) 0) (<=  (+ (- 75) skoX) 0)))
(check-sat)
