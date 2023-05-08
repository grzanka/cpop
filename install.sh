# load necessary modules
ml root/6.24.06-foss-2021b
ml cmake/3.22.1-gcccore-11.2.0
ml mpfr/4.1.0-gcccore-11.2.0
ml boost/1.79.0-gcc-11.2.0

# folder for installation of dependencies
THIS_SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
USR_DIR=$THIS_SCRIPT_DIR/usr

GEANT4_INSTALL=$USR_DIR/geant4_install
CGAL_INSTALL=$USR_DIR/cgal_install
CLHEP_INSTALL=$USR_DIR/CLHEP_install
XERCESC_INSTALL=$USR_DIR/xercesc_install

CPOP_INSTALL=$THIS_SCRIPT_DIR/cpop_install
CPOP_SOURCE=$THIS_SCRIPT_DIR
CPOP_BUILD=$THIS_SCRIPT_DIR/cpop_build

# check if CPOP install dir exists and if not then install
if [ ! -d $CPOP_INSTALL ]; then
    ml qt5/5.15.2-gcccore-11.2.0
    echo "Installing CPOP in ${CPOP_INSTALL}"
    echo "CGAL_INSTALL: ${CGAL_INSTALL}"
    echo "CLHEP_INSTALL: ${CLHEP_INSTALL}"
    echo "XERCESC_INSTALL: ${XERCESC_INSTALL}"
    cmake -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=$CPOP_INSTALL \
        -DCMAKE_PREFIX_PATH="${CGAL_INSTALL};${CLHEP_INSTALL};${XERCESC_INSTALL};${GEANT4_INSTALL}" \
         -S $CPOP_SOURCE -B $CPOP_BUILD
    cmake --build $CPOP_BUILD --target install --parallel
else
    echo "CPOP already installed in ${CPOP_INSTALL}"
fi
