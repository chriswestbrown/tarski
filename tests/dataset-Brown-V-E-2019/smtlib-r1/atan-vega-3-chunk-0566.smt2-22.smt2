(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoY () Real)
(declare-fun skoX () Real)
(declare-fun skoZ () Real)
(assert (and (<= (* (- 1) skoY) 0) (<= (* (- 1) (+ (+ (* skoY (* skoY (* skoY (+ (- 945) (* skoY (* skoY (- 483))))))) (* skoX (+ (* skoY (* skoY (+ (- 2835) (* skoY (* skoY (+ (- 2205) (* skoY (* skoY (- 192))))))))) (* skoX (+ (* skoY (+ (- 2835) (* skoY (* skoY (+ (- 3465) (* skoY (* skoY (- 836)))))))) (* skoX (+ (- 945) (* skoY (* skoY (+ (- 1995) (* skoY (* skoY (+ (- 960) (* skoY (* skoY (- 64)))))))))))))))) (* skoZ (+ (+ (- 2835) (* skoY (* skoY (+ (- 3150) (* skoY (* skoY (- 675))))))) (* skoX (+ (* skoY (+ 2835 (* skoY (* skoY (+ 3150 (* skoY (* skoY 675))))))) (* skoX (+ (+ (- 945) (* skoY (* skoY (+ (- 1050) (* skoY (* skoY (- 225))))))) (* skoX (* skoY (+ 945 (* skoY (* skoY (+ 1050 (* skoY (* skoY 225)))))))))))))))) 0) (<= (* (- 1) (+ (+ (+ 1 (* skoY (- 1))) (* skoX (+ (- 1) (* skoY (- 1))))) (* skoZ (+ (- 1) (* skoX skoY))))) 0) (<= (* (- 1) (+ (* skoX (+ (* skoY (* skoY (- 3))) (* skoX (+ (* skoY (- 3)) (* skoX (+ (- 1) (* skoY (* skoY (- 1))))))))) (* skoZ (+ (- 3) (* skoX (+ (* skoY 3) (* skoX (+ (- 1) (* skoX skoY))))))))) 0) (>  skoZ 0) (>  (+ 1 skoX) 0) (> (* (- 1) (+ (- 1) skoY)) 0) (> (* (- 1) (+ (* skoY (- 1)) skoX)) 0)))
(check-sat)
