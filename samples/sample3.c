#include "sample_common.h"

sg_image fbo_image;
sg_pass fbo_pass;

static sgp_vec2 points_buffer[4096];

void draw_triangles() {
    sgp_irect viewport = sgp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float w = height*0.3f;
    int count = 0;
    float step = (2.0f*M_PI)/6.0f;
    for(float theta = 0.0f; theta <= 2.0f*M_PI + step*0.5f; theta+=step) {
        points_buffer[count++] = (sgp_vec2){hw + w*cosf(theta), hh - w*sinf(theta)};
        if(count % 3 == 1)
            points_buffer[count++] = (sgp_vec2){hw, hh};
    }
    sgp_set_color(1.0f, 0.0f, 1.0f, 1.0f);
    sgp_draw_filled_triangle_strip(points_buffer, count);
}

void draw_fbo() {
    sgp_begin(128, 128);
    draw_triangles();

    sg_pass_action pass_action = {
        .colors = {{.action = SG_ACTION_CLEAR, .val = {0.0f, 0.0f, 0.0f, 0.0f}}},
        .depth = {.action = SG_ACTION_DONTCARE},
        .stencil = {.action = SG_ACTION_DONTCARE},
    };
    sg_begin_pass(fbo_pass, &pass_action);
    sgp_flush();
    sg_end_pass();
    sgp_end();
}

void draw(int width, int height) {
    float time = SDL_GetTicks() / 1000.0f;
    draw_fbo();
    for(int y=0;y<height;y+=128) {
        for(int x=0;x<width;x+=128) {
            sgp_push_transform();
            sgp_rotate_at(time, x+64, y+64);
            sgp_draw_textured_rect(fbo_image, (sgp_rect){(float)x, (float)y, 128, 128}, NULL);
            sgp_pop_transform();
        }
    }
}

bool init() {
    sg_image_desc fbo_image_desc = {
        .render_target = true,
        .width = 128,
        .height = 128,
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    };
    fbo_image = sg_make_image(&fbo_image_desc);
    if(fbo_image.id == SG_INVALID_ID)
        return false;
    sg_pass_desc pass_desc = {
        .color_attachments = {{.image = fbo_image}},
    };
    fbo_pass = sg_make_pass(&pass_desc);
    if(fbo_pass.id == SG_INVALID_ID)
        return false;
    return true;
}

void terminate() {
    sg_destroy_image(fbo_image);
    sg_destroy_pass(fbo_pass);
}

int main(int argc, char *argv[]) {
    return sample_app((sample_app_desc){
        .init = init,
        .terminate = terminate,
        .draw = draw,
        .argc = argc,
        .argv = argv
    });
}