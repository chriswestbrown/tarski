(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (<  (+ (+ (* skoX (* skoX 500000)) (* skoY (* skoY 500000))) (* skoZ (+ (+ (* skoX (- 40)) (* skoY 7)) (* skoZ 500200)))) 0) (>  (+ (+ (* skoX (- 2540)) (* skoY (- 493))) (* skoZ 17200)) 0) (<  (+ (+ (* skoX (+ 2540 (* skoX 50000))) (* skoY (+ 493 (* skoY 50000)))) (* skoZ (+ (+ (+ (- 17200) (* skoX (- 2290))) (* skoY (- 443))) (* skoZ 65500)))) 0) (<  (+ (+ (+ (- 1000) (* skoX (* skoX 26100))) (* skoY (+ (* skoX 10500) (* skoY 14100)))) (* skoZ (+ (+ (* skoX (- 2130)) (* skoY (- 413))) (* skoZ 7200)))) 0) (not (=  skoX 0))))
(check-sat)