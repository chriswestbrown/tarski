(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun pi () Real)
(assert (and (>  (+ (- 62768369664000) (* skoX (* skoX (+ 20922789888000 (* skoX (* skoX (+ (- 2789705318400) (* skoX (* skoX (+ 197707910400 (* skoX (* skoX (+ (- 8060532480) (* skoX (* skoX (+ 194725440 (* skoX (* skoX (+ (- 2949120) (* skoX (* skoX (+ 29844 (* skoX (* skoX (+ (- 212) (* skoX skoX)))))))))))))))))))))))))) 0) (not (=  skoX 0)) (>  (+ (* skoX (- 1)) skoY) 0) (>  (+ (- 1) (* skoX 10)) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (=  (+ (+ (- 2) (* skoY (- 10))) (* pi 5)) 0)))
(check-sat)
