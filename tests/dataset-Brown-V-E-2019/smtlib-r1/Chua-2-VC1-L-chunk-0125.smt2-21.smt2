(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(declare-fun skoX () Real)
(assert (and (<= (* (- 1) (+ (+ (+ 828000000000000000000000000 (* skoC 960000000000000000000000000)) (* skoS (- 1058400000000000000000000000))) (* skoX (+ 4802400000000000000000000 (* skoX (+ 13926960000000000000000 (* skoX (+ 26925456000000000000 (* skoX (+ 34161672300000000 (* skoX (+ 28305385620000 (* skoX 13680936383))))))))))))) 0) (> (* (- 1) (+ (+ (+ 828000000000000000000000000 (* skoC 960000000000000000000000000)) (* skoS (- 1058400000000000000000000000))) (* skoX (+ 4802400000000000000000000 (* skoX (+ 13926960000000000000000 (* skoX (+ 26925456000000000000 (* skoX (+ 34161672300000000 (* skoX (+ 28305385620000 (* skoX 13680936383))))))))))))) 0) (<= (* (- 1) skoX) 0) (<=  skoX 0) (>  (+ (+ (+ (- 103500000000) (* skoC (- 120000000000))) (* skoS 132300000000)) (* skoX (+ (+ (+ (- 300150000) (* skoC 348000000)) (* skoS (- 383670000))) (* skoX (+ (+ (- 290145) (* skoC (- 336400))) (* skoS 370881)))))) 0) (<= (* (- 1) (+ (* skoC (- 400)) (* skoS 441))) 0) (>  (+ (* skoC (- 400)) (* skoS 441)) 0) (=  (+ (+ (- 1) (* skoC skoC)) (* skoS skoS)) 0) (>  (+ (- 1570) (* skoX 21)) 0) (<= (* (- 1) skoC) 0) (<=  (+ (- 75) skoX) 0)))
(check-sat)
