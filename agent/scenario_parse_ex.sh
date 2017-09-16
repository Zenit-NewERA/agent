#!/bin/sh
flex++ -oscenario_tok.C scenario_tok.l
bison -d scenario_par.y -o scenario_par.c
rm scenario_par.c
bison scenario_par.y -o scenario_par.C
