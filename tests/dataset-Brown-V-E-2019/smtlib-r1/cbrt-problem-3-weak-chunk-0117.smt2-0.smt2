(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (>  (+ (+ (+ 5 (* skoX (+ (- 6) (* skoX (+ (- 3) (* skoX 4)))))) (* skoY (+ (+ (- 6) (* skoX (+ (- 130) (* skoX (+ (- 88) (* skoX 8)))))) (* skoY (+ (+ (- 3) (* skoX (+ (- 88) (* skoX 16)))) (* skoY (+ 4 (* skoX 8)))))))) (* skoZ (+ (+ (+ (- 6) (* skoX (+ (- 130) (* skoX (+ (- 88) (* skoX 8)))))) (* skoY (+ (+ (- 130) (* skoX (+ (- 668) (* skoX (+ (- 340) (* skoX 16)))))) (* skoY (+ (+ (- 88) (* skoX (+ (- 340) (* skoX 32)))) (* skoY (+ 8 (* skoX 16)))))))) (* skoZ (+ (+ (+ (- 3) (* skoX (+ (- 88) (* skoX 16)))) (* skoY (+ (+ (- 88) (* skoX (+ (- 340) (* skoX 32)))) (* skoY (+ 16 (* skoX 32)))))) (* skoZ (+ (+ 4 (* skoX 8)) (* skoY (+ 8 (* skoX 16)))))))))) 0) (> (* (+ 1 (* skoY 2)) (+ (+ (+ 4 (* skoX 5)) (* skoY 5)) (* skoZ 5)) (+ 1 (* skoZ 2))) 0) (>  (+ (- 1) (* skoZ 20)) 0) (>  (+ (- 1) (* skoY 20)) 0) (>  (+ (- 1) (* skoX 20)) 0) (> (* (- 1) (+ (- 15) skoZ)) 0) (> (* (- 1) (+ (- 15) skoY)) 0) (> (* (- 1) (+ (- 15) skoX)) 0)))
(check-sat)
