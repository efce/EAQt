# EAQt Electrochemical Analuzer Software based on Qt API
This software is designed ot be used with MTM-ANKO M161, and 8KCA electrochemical
software analyzers. It uses its own file extenstion for handling the measurement 
result ".volt". The VOLT file format is described in seperate file VOLT_File.md.
However, it is able to load .vol file of EALab and EAGraph.
The software is based around QCustomPlot (http://www.qcustomplot.com) GPLv3 library
for creating plots, and it also makes use of Eigen BSD library
(http://eigen.tuxfamily.org/) for signals processing.Both sourc codes are included.

# Developement and compilation
This program is developed with QtCreator for Qt >= 5.8. Every dependiency is included
in the repository, and it should be compatible with Windows, Linux and MacOS. No
additional dependiencies should be required.

# Instlation
As for the moment no compiled form is available, as software is in heavy development
and possibly contains bugs which may result in the loss of data.
