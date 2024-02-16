cmake_minimum_required(VERSION 3.28)

include(pico_sdk_import.cmake)
project(graphics_text_4bit)

add_executable(graphics_text_4bit)


target_sources(graphics_text_4bit PRIVATE 
	vga16_graphics_font.c 
	vga16_graphics.c
)

target_link_libraries(graphics_text_4bit PRIVATE 
	pico_stdlib 
	pico_bootsel_via_double_reset 
	hardware_pio 
	hardware_dma 
	hardware_adc 
	hardware_sync
	hardware_irq
	pico_multicore
)

pico_sdk_init()


# must match with pio filename and executable name from above
pico_generate_pio_header(graphics_text_4bit ${CMAKE_CURRENT_LIST_DIR}/hsync.pio)
pico_generate_pio_header(graphics_text_4bit ${CMAKE_CURRENT_LIST_DIR}/vsync.pio)
pico_generate_pio_header(graphics_text_4bit ${CMAKE_CURRENT_LIST_DIR}/rgb.pio)


pico_add_extra_outputs(graphics_text_4bit)

add_compile_options(-Ofast)
