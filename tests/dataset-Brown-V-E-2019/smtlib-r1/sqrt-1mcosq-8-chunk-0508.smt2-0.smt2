(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun pi () Real)
(declare-fun skoY () Real)
(assert (and (> (* (- 1) (+ 20922789888000 (* skoX (* skoX (+ (- 10461394944000) (* skoX (* skoX (+ 871782912000 (* skoX (* skoX (+ (- 29059430400) (* skoX (* skoX (+ 518918400 (* skoX (* skoX (+ (- 5765760) (* skoX (* skoX (+ 43680 (* skoX (* skoX (+ (- 240) (* skoX skoX)))))))))))))))))))))))) 0) (<=  (+ (+ 2 (* pi (- 5))) (* skoY 10)) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (> (* (- 1) (+ (- 31415927) (* pi 10000000))) 0) (<= (* (- 1) (+ (- 1) (* skoX 10))) 0) (>  (+ (* skoX (- 1)) skoY) 0)))
(check-sat)
