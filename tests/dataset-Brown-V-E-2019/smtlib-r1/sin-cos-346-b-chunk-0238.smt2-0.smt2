(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoSQ3 () Real)
(declare-fun skoX () Real)
(declare-fun pi () Real)
(assert (and (>  skoSQ3 0) (> (* skoSQ3 skoSQ3) 0) (>  (+ (+ (- 4311014400) (* skoSQ3 (* skoSQ3 1437004800))) (* skoX (* skoX (+ (+ 718502400 (* skoSQ3 (* skoSQ3 (- 239500800)))) (* skoX (* skoX (+ (* skoSQ3 (* skoSQ3 19958400)) (* skoX (* skoX (+ (* skoSQ3 (* skoSQ3 (- 665280))) (* skoX (* skoX (+ (* skoSQ3 (* skoSQ3 11880)) (* skoX (* skoX (+ (* skoSQ3 (* skoSQ3 (- 132))) (* skoX (* skoX (* skoSQ3 skoSQ3))))))))))))))))))) 0) (>  (+ (+ (- 1) (* skoX (- 10000000))) (* pi 5000000)) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (> (* (- 1) (+ (- 31415927) (* pi 10000000))) 0) (>  skoX 0)))
(check-sat)
