section .rodata

global mtcnn_r_param

mtcnn_r_param:
	align 32
	incbin "mtcnn_r.param"

