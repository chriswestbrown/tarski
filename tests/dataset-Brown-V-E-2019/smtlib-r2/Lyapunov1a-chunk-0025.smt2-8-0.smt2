(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoZ () Real)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(assert (and (<  (+ (- 10) (* skoZ 9)) 0) (>  (+ (+ (* skoZ (- 17200)) (* skoX 2540)) (* skoY 493)) 0) (<  (+ (+ (* skoZ (+ (- 17200) (* skoZ 65500))) (* skoX (+ (+ 2540 (* skoZ (- 2290))) (* skoX 50000)))) (* skoY (+ (+ 493 (* skoZ (- 443))) (* skoY 50000)))) 0) (<  (+ (+ (+ (- 1000) (* skoZ (* skoZ 7200))) (* skoX (+ (* skoZ (- 2130)) (* skoX 26100)))) (* skoY (+ (+ (* skoZ (- 413)) (* skoX 10500)) (* skoY 14100)))) 0) (not (=  skoZ 0))))
(check-sat)