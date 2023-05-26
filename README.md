# EasyMon

Mini system monitor software for Linux.


## I. Author

C.T. Bishop - https://github.com/ctbishop - [Contact Me](https://casandrabishop.com/contact/)


## II. Disclaimer and Notice

"This Software" includes all files in the EasyMon repository that is
accessible at https://github.com/ctbishop/EasyMon. The "Developer" is the
primary author and originator of This Software (i.e., C.T. Bishop -
https://github.com/ctbishop), while "User" refers to as any person(s) or
entity other than the Developer who acquires or uses This Software or any
component file(s), such as by download, installation, copying, modification,
redistribution, compiling, executation, or other method of acquisition or
utilization.

This Software is intended for non-commercial use only and is offered by the
Developer free of charge, "as-is", and with no warranties or guarantees. The
User is responsible for complying with all relevant laws and regulations for
the use of This Software and assumes all associated risks. The Developer
will not be liable for any damage, loss, complication, or other harm caused
or precipitated by any action(s) and/or lack thereof of the User in using
This Software, even when advised by the Developer.

This Software is designed to be used in conjunction with the GTK 4 library,
which is licensed under the terms of version 2.1+ of the GNU Lesser General
Public License. However, the Developer is not affiliated with the GTK
Project. For the official GTK Project website and documentation, visit
https://gtk.org/. 


## III. Table of Contents

1. About  
2. Compatibility  
3. How to Install and Launch EasyMon
4. Uninstalling EasyMon
5. Troubleshooting and Modifications


## 1. About

EasyMon is a desktop application which summarizes and regularly updates
information regarding your use of hardware resources.

### Dimensions
The user interface consists of a single window 300 pixels wide. The total
height of the window varies according to your hardware, system settings, and
whether or not any expandable areas are open. As a general guide, this is
220 px fully collapsed, and 370 to 640 px fully expanded. 

### Features
The application window contains a "Monitor" page and a "Settings" page which
can be switched between via their respective tab buttons near the top of
the window.

The Monitor page features two expandable areas, one corresponding to
computing resources, titled "CPU," and the other to memory resources, titled
"MEM." Within these titles, overall (aggregate) cpu usage and total system
memory usage are respectively displayed. Opening either expander reveals
additional details. The following abbreviations are used:

Cor - physical core number  
Prc - processor/thread number  
Use% - use percent of component  
MHz - frequency (MHz) of component  
buff - buffers usage  
page - page cache usage  
swap - swap cache usage  
installed - total system memory  

The settings page contains controls for update interval, memory display
units, and dark mode (color scheme). "Update interval" indicates the time
interval between updates of displayed information, where options include
0.5, 1, and 3 seconds. Memory values are shown in gigabytes by default, but
can be switched to "%" to show them as a percentage of total system memory.
Regardless of settings, "installed" memory is always shown in gigabytes.


## 2. Compatibility

EasyMon should be compatible with most common and modern Linux
distributions, but is not compatible with Windows or Mac operating systems.
This is because the program depends on the existence and accessibility of
the following virtual files in /proc: cpuinfo, stat, and meminfo. Outdated
Linux distributions, in particular older versions of Red Hat, may not be
compatible due to differences in meminfo.


## 3. How to Install and Launch EasyMon

### Installing GTK 4
Before before compiling EasyMon, please ensure the GTK 4 development library
and its dependencies are installed. This can be done using one of the
following commands.

For Ubuntu, Mint, or other Debian distributions:

    sudo apt install libgtk-4-dev

For Red Hat, CentOS, and other RPM-based distributions:

    sudo dnf install gtk4-devel

Should you encounter issues or require additional support, please refer to
GTK's documentation on the matter:
https://www.gtk.org/docs/installations/linux/

### Download, Compile, and Install Easymon
Once you have installed the GTK 4 development library and its dependencies,
you can proceed the the following steps. First, open a terminal and navigate
to an appropriate directory to which you will download the EasyMon
installation package. For example:

    cd ~/Downloads

Then, download the package from GitHub:

    wget https://github.com/ctbishop/EasyMon/archive/main.zip

Unzip the files to a new directory:

    unzip -j main.zip -d EasyMon

Navigate to the new directory:

    cd EasyMon

Then, compile the program:

    make

Once the program is compiled, give the installer script permission to execute:

    chmod +x easymon-install-reinstall.sh

and run the installer:

    ./easymon-install-reinstall.sh

From here, simply respond to any of the installer's prompts and read its
output carefully. Following successful installation, you are free to discard
the downloaded zip file as well as the EasyMon directory to which you
extracted its contents, as these are no longer needed.

### Launch EasyMon
Following successful installation, EasyMon can be launched by clicking its
desktop icon, or from a terminal by typing:

    easymon


## 4. Uninstalling EasyMon

Installation of EasyMon includes an uninstaller. To use it, open a terminal
and enter:

    easymon-uninstall.sh

Follow the uninstaller's prompts and read its output carefully. The
uninstaller will remove the installed files as well as itself.


## 5. Troubleshooting and Modifications

### Resizability
The application window is not user-resizable. This means that you cannot
click and drag the edges to resize the window. This makes it possible for
the window to decrease in height when an open expander is closed. If this is
of any concern to you, you can change it by editing the window's "resizable"
property from FALSE to TRUE in the following line of easymon.ui.

    <property name="resizable">FALSE</property>

This will be near the top of the file.
