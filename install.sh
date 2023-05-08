# load necessary modules
ml cmake
ml gcc

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

