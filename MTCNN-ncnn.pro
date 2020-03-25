TEMPLATE = subdirs

SUBDIRS += libmtcnn
SUBDIRS += mtcnn
SUBDIRS += mtcnn-stdout
SUBDIRS += mtcnn-window

mtcnn-stdout.depends = libmtcnn
mtcnn-window.depends = mtcnn
