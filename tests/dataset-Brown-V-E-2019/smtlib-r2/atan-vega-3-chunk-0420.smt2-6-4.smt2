(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoY () Real)
(declare-fun skoX () Real)
(declare-fun skoZ () Real)
(assert (and (=  skoY 0) (=  (+ (+ (+ 1 (* skoY (* skoY 3))) (* skoX (+ (* skoY 4) (* skoX (+ 3 (* skoY skoY)))))) (* skoZ (+ (+ (* skoY 6) (* skoX (+ (+ 6 (* skoY (* skoY (- 6)))) (* skoX (* skoY (- 6)))))) (* skoZ (+ 3 (* skoX (+ (* skoY (- 6)) (* skoX (* skoY (* skoY 3)))))))))) 0) (>  (+ (+ (+ 79 (* skoY (+ 200 (* skoY (+ 237 (* skoY 150)))))) (* skoX (+ (+ 150 (* skoY (+ 316 (* skoY 50)))) (* skoX (+ 237 (* skoY (* skoY (+ 79 (* skoY 50))))))))) (* skoZ (+ (+ (+ 150 (* skoY (+ 474 (* skoY 300)))) (* skoX (+ (+ 474 (* skoY (* skoY (+ (- 474) (* skoY (- 300)))))) (* skoX (* skoY (+ (- 474) (* skoY (- 150)))))))) (* skoZ (+ (+ 237 (* skoY 150)) (* skoX (+ (* skoY (+ (- 474) (* skoY (- 300)))) (* skoX (* skoY (* skoY (+ 237 (* skoY 150)))))))))))) 0) (<  (+ (+ (* skoY (- 1)) (* skoX (- 1))) (* skoZ (+ (- 1) (* skoX skoY)))) 0) (=  (+ (+ (+ 1 (* skoY (- 1))) (* skoX (+ (- 1) (* skoY (- 1))))) (* skoZ (+ (- 1) (* skoX skoY)))) 0) (>  skoZ 0) (>  (+ 1 skoX) 0) (<  (+ (- 1) skoY) 0) (<  (+ (* skoY (- 1)) skoX) 0)))
(check-sat)
