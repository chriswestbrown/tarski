(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun pi () Real)
(declare-fun skoY () Real)
(assert (and (=  (+ (- 1440) (* skoX (* skoX (+ 420 (* skoX (* skoX (+ (- 32) (* skoX skoX)))))))) 0) (=  (+ (+ 2 (* pi (- 5))) (* skoY 10)) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (=  (+ (- 1) (* skoX 10)) 0) (>  (+ (* skoX (- 1)) skoY) 0)))
(check-sat)