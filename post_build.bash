#!/bin/bash
sudo yum -y remove fontventa
sudo yum localinstall --nogpgcheck -y fontventa*.rpm
IVES_convert.ksh -i PUB_Guerlain_-_La_petite_robe_noire.mp4 -o test_convert.mp4
ENVOYER_VISIO2=$1
if [ "$ENVOYER_VISIO2" != "" ] 
then
    echo "On envoie sur visio2"
    scp -oPort=2222 *.rpm jenkins@visio2.visioassistance.net:/home/jenkins/$NODE_NAME
else
    echo "On n'envoie rien sur visio2"
fi
