(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoSM () Real)
(declare-fun skoSS () Real)
(declare-fun skoX () Real)
(assert (and (<  (+ (+ (+ (- 244992) (* skoSM 244992)) (* skoSS (+ (- 122496) (* skoSM 122496)))) (* skoX (+ (+ (+ (- 169984) (* skoSM 431488)) (* skoSS (+ (- 244992) (* skoSM 183744)))) (* skoX (+ (+ (+ 205760 (* skoSM 249120)) (* skoSS (+ (- 153120) (* skoSM 76560)))) (* skoX (+ (+ (+ 178752 (* skoSM 55312)) (* skoSS (+ (- 30624) (* skoSM 7656)))) (* skoX (+ (+ (+ 30086 (* skoSM 4000)) (* skoSS (- 957))) (* skoX 500)))))))))) 0) (=  (+ (+ (+ (- 3828) (* skoSM 3828)) (* skoSS (+ (- 1914) (* skoSM 1914)))) (* skoX (+ (+ (+ 3086 (* skoSM 1000)) (* skoSS (- 957))) (* skoX 500)))) 0) (<  (+ (- 1) skoX) 0) (>  skoX 0) (>  skoSM 0) (=  skoSS 0) (=  (+ (+ (- 1) (* skoSM skoSM)) skoX) 0) (=  (+ (+ (- 1) (* skoSS skoSS)) (* skoX skoX)) 0)))
(check-sat)