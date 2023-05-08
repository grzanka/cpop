# load necessary modules
ml root/6.24.06-foss-2021b
ml cmake/3.22.1-gcccore-11.2.0
ml mpfr/4.1.0-gcccore-11.2.0

# folder for installation of dependencies
THIS_SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
USR_DIR=$THIS_SCRIPT_DIR/usr
mkdir --parents $USR_DIR
cd $USR_DIR

# install cgal
# check if cgal directory exists and if not then clone
CGAL_SOURCE=$USR_DIR/cgal
CGAL_BUILD=$USR_DIR/cgal_build
CGAL_INSTALL=$USR_DIR/cgal_install
if [ ! -d $CGAL_SOURCE ]; then
    git clone --branch v5.5.1 --depth 1 https://github.com/CGAL/cgal
else
    echo "CGAL already cloned in ${CGAL_SOURCE}"
fi
mkdir --parents $CGAL_BUILD
cd $CGAL_BUILD

# check if cgal_install directory exists and if not then install
if [ ! -d $CGAL_INSTALL ]; then
    echo "Installing CGAL in ${CGAL_INSTALL}"
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$CGAL_INSTALL -S $CGAL_SOURCE -B $CGAL_BUILD
    cmake --build $CGAL_BUILD --target install --parallel
else
    echo "CGAL already installed in ${CGAL_INSTALL}"
fi
cd $USR_DIR

# install CLHEP
# check if CLHEP directory exists and if not then clone
CLHEP_SOURCE=$USR_DIR/CLHEP
CLHEP_BUILD=$USR_DIR/CLHEP_build
CLHEP_INSTALL=$USR_DIR/CLHEP_install
if [ ! -d $CLHEP_SOURCE ]; then
    git clone --branch CLHEP_2_4_6_1 --depth 1 https://gitlab.cern.ch/CLHEP/CLHEP.git
else
    echo "CLHEP already cloned in ${CLHEP_SOURCE}"
fi
mkdir --parents $CLHEP_BUILD
cd $CLHEP_BUILD

# check if CLHEP_install directory exists and if not then install
if [ ! -d $CLHEP_INSTALL ]; then
    echo "Installing CLHEP in ${CLHEP_INSTALL}"
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$CLHEP_INSTALL -S $CLHEP_SOURCE -B $CLHEP_BUILD
    cmake --build $CLHEP_BUILD --target install --parallel
else
    echo "CLHEP already installed in ${CLHEP_INSTALL}"
fi
cd $USR_DIR

# XercesC

# check if XercesC directory exists and if not then clone
XERCESC_SOURCE=$USR_DIR/xerces-c
XERCESC_BUILD=$USR_DIR/xercesc_build
XERCESC_INSTALL=$USR_DIR/xercesc_install
if [ ! -d $XERCESC_SOURCE ]; then
    git clone --branch v3.2.4 --depth 1 https://github.com/apache/xerces-c.git
else
    echo "XercesC already cloned in ${XERCESC_SOURCE}"
fi

mkdir --parents $XERCESC_BUILD
cd $XERCESC_BUILD
if [ ! -d $XERCESC_INSTALL ]; then
    echo "Installing XercesC in ${XERCESC_INSTALL}"
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$XERCESC_INSTALL -S $XERCESC_SOURCE -B $XERCESC_BUILD
    cmake --build $XERCESC_BUILD --target install --parallel
else
    echo "XercesC already installed in ${XERCESC_INSTALL}"
fi
cd $USR_DIR


# Geant4
# check if Geant4 directory exists and if not then clone
GEANT4_SOURCE=$USR_DIR/geant4
GEANT4_BUILD=$USR_DIR/geant4_build
GEANT4_INSTALL=$USR_DIR/geant4_install

if [ ! -d $GEANT4_SOURCE ]; then
    git clone --branch v11.1.0 --depth 1 https://github.com/Geant4/geant4.git
else
    echo "Geant4 already cloned in ${GEANT4_SOURCE}"
fi

mkdir --parents $GEANT4_BUILD
cd $GEANT4_BUILD

# check if Geant4_install directory exists and if not then install
if [ ! -d $GEANT4_INSTALL ]; then
    echo "Installing Geant4 in ${GEANT4_INSTALL}"
    cmake -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${GEANT4_INSTALL}" \
    -DGEANT4_USE_SYSTEM_CLHEP=OFF \
    -DCMAKE_PREFIX_PATH="${CLHEP_INSTALL};${XERCESC_INSTALL}" \
    -DGEANT4_INSTALL_DATA=ON \
    -DGEANT4_USE_GDML=ON \
    -S $GEANT4_SOURCE -B $GEANT4_BUILD
    cmake --build $GEANT4_BUILD --target install --parallel
else
    echo "Geant4 already installed in ${GEANT4_INSTALL}"
fi  
cd $USR_DIR