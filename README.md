# EAQt Electrochemical Analyzer Software based on Qt API
This software is designed to be used with MTM-ANKO M161, and 8KCA electrochemical analyzers.
It uses its own file extenstion for handling the measurement result ".volt". The VOLT file
format is described in seperate file (VOLT_File.md). However, it is able to load .vol file
of EALab and EAGraph. This program makes use of following software:
- Qt API (https://www.qt.io) (LGPLv3 license).
- QCustomPlot (http://www.qcustomplot.com) library for displaying plots (GPLv3 license).
- Eigen library (http://eigen.tuxfamily.org) for matrix operations and signals processing (MPL 2.0 license).
- kissFFT (https://sourceforge.net/projects/kissfft/) for FFT and iFFT (revised BSD license).
- pstdin.h (http://mrpt.sourceforge.net/reference/0.6.5/pstdint_8h-source.html) for compatibilty on Windows (revised BSD license).
- LiberationSans font (https://pagure.io/liberation-fonts/) (SIL OPEN FONT license).

# Developement and compilation
This program is developed with QtCreator for Qt >= 5.8. Every dependiency is included
in the repository, and it should be compatible with Windows, Linux and MacOS. No
additional dependiencies should be required.

# Instlation
As for the moment no official binary form is available, as the software is under heavy development
and possibly contains bugs, which may result in the loss of data.

# License
EAQt is licensed under GPLv3. Licensed are avaiable in About->Licenses, or with the source code.

# Author
This program is mainly developed by Filip Ciepiela <filip.ciepiela@agh.edu.pl> and Małgorzata Jakubowska <jakubows@agh.edu.pl>, however every commit is welcome.
