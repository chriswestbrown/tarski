(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoY () Real)
(declare-fun skoX () Real)
(declare-fun pi () Real)
(assert (and (>  (+ 479001600 (* skoY (* skoY (+ (- 239500800) (* skoY (* skoY (+ 19958400 (* skoY (* skoY (+ (- 665280) (* skoY (* skoY (+ 11880 (* skoY (* skoY (+ (- 132) (* skoY skoY))))))))))))))))) 0) (>  (+ (- 3628800) (* skoY (* skoY (+ 1814400 (* skoY (* skoY (+ (- 151200) (* skoY (* skoY (+ 5040 (* skoY (* skoY (+ (- 90) (* skoY skoY)))))))))))))) 0) (=  (+ (- 720) (* skoY (* skoY (+ 360 (* skoY (* skoY (+ (- 30) (* skoY skoY)))))))) 0) (=  (+ (- 2) (* skoY skoY)) 0) (<  (+ (* skoY (- 1)) (* skoX 10)) 0) (=  (+ (- 1) (* skoX 20)) 0) (=  (+ (* skoY (- 2)) pi) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (>  (+ (- 15707963) (* pi 5000000)) 0)))
(check-sat)
