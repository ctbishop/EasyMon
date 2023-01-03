#!/bin/bash

##########################################################################################
#                                         About                                          #
##########################################################################################
#  Deinstallation script for "EasyMon" System Monitor app.                               #
#                                                                                        #
#     Author: C. Bishop
#     Origin: https://github.com/ctbishop/EasyMon                                        #
#                                                                                        #
#     PLEASE NOTE:                                                                       #
#     Although written with the best intentions, this script is not guaranteed to        #
#     function as intended or to produce the desired result. The execution of this       #
#     script is thus at your own risk.                                                      #
#                                                                                        #
##########################################################################################


app='EasyMon'
program='easymon'
warning=0

## (EXPECTED) INSTALLED FILES.
declare -a files
    files[0]='/usr/bin/easymon'
    files[1]='/usr/share/applications/com.gmail.ctbishop34.EasyMon.desktop'
    files[2]='/usr/share/icons/hicolor/scalable/apps/easymon-icon.svg'


## CHECK/GET ROOT PRIVILEGES.
echo -e "\n$app un-installer.\n"  #informative statement.
if [ ! $(sudo echo 0) ];then
    exit
fi


## PROMPT TO UNINSTALL.
echo -e -n "\nUninstall $app completely? (y/n) "
read  #prompt
until [[ $REPLY = 'y' || $REPLY = 'n' ]];do
    echo -n "Enter y or n: "
    read  #re-prompt
done
if [ $REPLY = 'n' ];then
    echo -e "\nExiting...\n"
    exit
fi


## CHECK IF CURRENTLY RUNNING.
pid=$(ps -e | grep $program | awk '{print $0" "}' | grep " easymon " | awk '{print $1}')  #process ID
if [ ! -z $pid ];then  #not empty
    echo -e "\n$app is currently running (pid $pid)."
    echo "Please exit $app before re-starting the uninstaller."
    echo -e "\nExiting...\n"
    exit
fi


## UNINSTALL FILES.
echo -e "\nUninstalling files..."
for ((a=0;a<3;a++));do
    if [ -f ${files[${a}]} ];then  #file found
        if sudo rm ${files[${a}]} && [ ! -f ${files[${a}]} ];then  #remove (success & re-confirm)
            echo -e "\t removed: ${files[${a}]}"
        else
            echo -e "\tWARNING! Could not remove file: ${files[${a}]}"
            warning=1
        fi
    else echo -e "\t not found: ${files[${a}]}"  #inform only - no warning
    fi
done


## PROMPT TO COMPLETE.
if [ $warning -eq 1 ];then
    echo -e "\n\nIssue(s) occurred during deinstallation. Please review the warning messages above.\n"
else echo -e "\n\nFiles were successfully removed.\n"
fi
echo -n "The final step will remove this uninstaller. Proceed? (y/n) "
read  #prompt
until [[ $REPLY = 'y' || $REPLY = 'n' ]];do
    echo -n "Enter y or n: "
    read  #re-prompt
done
if [ $REPLY = 'y' ];then
    trap "sudo rm ${0}" EXIT  #remove self on exit
    echo -e "\n\nUninstallation is complete!\nThank you for trying $app.\n"
else echo -e "\n\nExiting...\n"
fi


#end
