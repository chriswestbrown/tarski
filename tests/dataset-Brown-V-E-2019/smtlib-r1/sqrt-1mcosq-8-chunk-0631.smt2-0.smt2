(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun pi () Real)
(assert (and (<= (* (+ (- 174356582400) (* skoX (* skoX (+ 50854003200 (* skoX (* skoX (+ (- 3874590720) (* skoX (* skoX (+ 125405280 (* skoX (* skoX (+ (- 2210208) (* skoX (* skoX (+ 24388 (* skoX (* skoX (+ (- 184) (* skoX skoX)))))))))))))))))))) skoX skoX) 0) (>  (+ (* skoX (- 1)) skoY) 0) (<= (* (- 1) (+ (- 1) (* skoX 10))) 0) (> (* (- 1) (+ (- 31415927) (* pi 10000000))) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (<= (* (- 1) (+ (+ (- 2) (* skoY (- 10))) (* pi 5))) 0)))
(check-sat)
