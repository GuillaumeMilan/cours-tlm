#! /bin/echo Non, il ne faut pas executer ce fichier, mais faire: source

export SYSTEMCROOT=/matieres/5MMMTSP/tlm/systemc-2.3.1/

# Chaine de cross-compilation MicroBlaze, pour le TP3
mblaze=/matieres/5MMMTSP/tlm/microblaze/setup.sh
xilinx=/matieres/5MMMTSP/Xilinx/Vivado/2016.2/settings64.sh
if [ -f "$mblaze" ]; then
    source "$mblaze"
fi
if [ -f "$xilinx" ]; then
    source "$xilinx"
fi
