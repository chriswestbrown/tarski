(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(assert (and (> (* (+ (- 31250000000000) (* skoX (+ 3750000000 (* skoX (+ (- 225000) (* skoX 9)))))) (+ (- 31250000000000) (* skoX (+ 3750000000 (* skoX (+ (- 225000) (* skoX 9))))))) 0) (>  (+ (+ (+ (- 26367187500000000000000000000) (* skoX (+ 6328125000000000000000000 (* skoX (+ (- 759375000000000000000) (* skoX (+ 60750000000000000 (* skoX (+ (- 3189375000000) (* skoX (+ 109350000 (* skoX (- 2187))))))))))))) (* skoC (- 21606445312500000000000000000))) (* skoS 8410644531250000000000000000)) 0) (> (* (- 1) (+ (* skoC (- 1770)) (* skoS 689))) 0)))
(check-sat)
