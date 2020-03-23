section .rodata

global mtcnn_o_param

mtcnn_o_param:
	align 32
	incbin "mtcnn_o.param"

