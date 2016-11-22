# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2011-2015 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2011-2015 Lawrence Livermore National Laboratories, United States of America
#   2013-2015 RWTH Aachen University, Federal Republic of Germany
#
# See the file LICENSE.txt in the package base directory for details
#
# @file FindOmpt.cmake
#       Detects the ompt installation.
#
# @author Joachim Protze
# @date 01/26/2016

#
# -DOMPT_INSTALL_PREFIX=<ompt_prefix>
#
# tries to detect the installation of ompt
#

include(FindPackageHandleStandardArgs)

# use an explicitly given ompt path first
FIND_PATH(OMPT_INCLUDE_PATH ompt.h
            PATHS ${OMPT_INSTALL_PREFIX} ${OMPT_HOME} ${OMPD_INSTALL_PREFIX} ${OMPD_HOME} /usr /usr/local
            PATH_SUFFIXES include NO_DEFAULT_PATH)
# if not-found, try again at cmake locations
FIND_PATH(OMPT_INCLUDE_PATH ompt.h)

find_package_handle_standard_args(OMPT DEFAULT_MSG OMPT_INCLUDE_PATH )
