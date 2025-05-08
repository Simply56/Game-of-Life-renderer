use minifb::{Window, WindowOptions};
use std::slice;
use std::sync::Mutex;

static mut WINDOW: Option<Mutex<Window>> = None;

/// Initialize window (call once from C)

#[unsafe(no_mangle)]
pub extern "C" fn init_window(width: usize, height: usize) {
    let win = Window::new("minifb from C", width, height, WindowOptions::default())
        .expect("Unable to create window");

    unsafe {
        WINDOW = Some(Mutex::new(win));
    }
}

/// Render a frame
///
/// `pixels_ptr` is a pointer to a u32 array with `width * height` pixels
#[unsafe(no_mangle)]
pub extern "C" fn render_frame(pixels_ptr: *const u32, width: usize, height: usize) {
    if pixels_ptr.is_null() {
        return;
    }

    let buffer: &[u32] = unsafe { slice::from_raw_parts(pixels_ptr, width * height) };

    unsafe {
        if let Some(ref win_mutex) = WINDOW {
            let mut win = win_mutex.lock().unwrap();
            if win.is_open() && !win.is_key_down(minifb::Key::Escape) {
                win.update_with_buffer(buffer, width, height)
                    .expect("Failed to update buffer");
            }
        }
    }
}
