(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun pi () Real)
(declare-fun skoY () Real)
(assert (and (>  (+ (- 3628800) (* skoX (* skoX (+ 1814400 (* skoX (* skoX (+ (- 151200) (* skoX (* skoX (+ 5040 (* skoX (* skoX (+ (- 90) (* skoX skoX)))))))))))))) 0) (>  (+ (- 7257600) (* skoX (* skoX (+ 2116800 (* skoX (* skoX (+ (- 161280) (* skoX (* skoX (+ 5220 (* skoX (* skoX (+ (- 92) (* skoX skoX)))))))))))))) 0) (not (=  skoX 0)) (=  (+ (+ 2 (* pi (- 5))) (* skoY 10)) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (=  (+ (- 1) (* skoX 10)) 0) (>  (+ (* skoX (- 1)) skoY) 0)))
(check-sat)
