(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(declare-fun skoX () Real)
(assert (and (<  (+ (+ (+ 21573674414899200000000000000000000000000000000000000 (* skoC (- 375723739054080000000000000000000000000000000000000))) (* skoS 8806593620622075494400000000000000000000000000000000)) (* skoX (+ (+ (+ (- 478240004741529600000000000000000000000000000000000) (* skoC 10407547571798016000000000000000000000000000000000)) (* skoS (- 243942643291231491194880000000000000000000000000000))) (* skoX (+ (+ (+ 5235177729451622400000000000000000000000000000000 (* skoC (- 144144533869402521600000000000000000000000000000))) (* skoS 3378605609583556153049088000000000000000000000000)) (* skoX (+ (+ (+ (- 38329937505445478400000000000000000000000000000) (* skoC 1330934529394149949440000000000000000000000000)) (* skoS (- 31195791795154835146486579200000000000000000000))) (* skoX (+ (+ (+ 143503605517978828800000000000000000000000000 (* skoC (- 8064631414047677349888000000000000000000000))) (* skoS 189027000908766329215742115840000000000000000)) (* skoX (+ (+ (+ (- 195523703851122524160000000000000000000000) (* skoC 31912898595588666084556800000000000000000)) (* skoS (- 748006846453261045610865229824000000000000))) (* skoX (+ (+ (+ (- 768276598709989177344000000000000000000) (* skoC (- 73665607591483837545185280000000000000))) (* skoS 1726649137229610913618413905510400000000)) (* skoX (+ 5963954165527998062592000000000000000 (* skoX (+ 28166850170371622587392000000000000 (* skoX (+ 75542536022051169040128000000000 (* skoX (+ 116760353815670304164496000000 (* skoX (+ 110589472712928899846136000 (* skoX 66872202985390093101721)))))))))))))))))))))))) 0) (=  skoX 0) (<  (+ (* skoC (- 86400000)) (* skoS 2025130727)) 0) (=  (+ (+ (- 1) (* skoC skoC)) (* skoS skoS)) 0)))
(check-sat)