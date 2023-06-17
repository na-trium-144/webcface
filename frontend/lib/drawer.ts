export const Colors = {
  red: [255, 0, 0],
  blue: [0, 0, 255],
  green: [0, 255, 0],
  yellow: [255, 255, 0],
  black: [0, 0, 0],
  white: [255, 255, 255],
  gray: [100, 100, 100],
  orange: [255, 165, 0],
  dark_red: [100, 0, 0],
  dark_red2: [150, 0, 0],
  dark_blue: [0, 0, 100],
  dark_blue2: [0, 0, 150],
  dark_green: [0, 100, 0],
  dark_yellow: [150, 150, 0],
  dark_black: [0, 0, 0],
  dark_gray: [100, 100, 100],
} as const;

export type Color = typeof Colors[keyof typeof Colors]

export class myPlot {
  canvas: any;
  gl: any;
  vertices: number[];
  colors: number[];
  height: number;
  width: number;
  vertex_buffer: any;
  color_buffer: any;
  matrix_buffer: any;
  vertshader: any;
  fragshader: any;
  shaderprogram: any;

  constructor(id: string) {
    this.canvas = document.getElementById(id);
    this.gl = this.canvas.getContext('experimental-webgl');

    this.vertices = [];
    this.colors = [];

    this.height = 800;
    this.width = 600;

    this.vertex_buffer = this.gl.createBuffer();
    this.color_buffer = this.gl.createBuffer();
    this.matrix_buffer = this.gl.createBuffer();
  }

  reset_canvas_id(id: string) {
    this.canvas = document.getElementById(id);
    this.gl = this.canvas.getContext('experimental-webgl');
    this.set_field_size(12000, 12000);
    this.create_shader();
  }

  set_field_size(w: number, h: number) {
    this.width = w;
    this.height = h;
  }

  create_shader() {
    const vertcode =
      'attribute vec2 vertex;' +
      'attribute vec3 color;' +
      'varying lowp vec4 vcolor;' +
      'uniform mat4 view_matrix;' +
      'void main(void) {' +
      'gl_Position= view_matrix * vec4(vertex, 0.0, 1.0);' +
      'gl_Position.w = 1.0;' +
      'gl_PointSize = 5.0;' +
      'vcolor = vec4(color / 255., 1.0);' +
      '}';
    this.vertshader = this.gl.createShader(this.gl.VERTEX_SHADER);
    this.gl.shaderSource(this.vertshader, vertcode);
    const res = this.gl.compileShader(this.vertshader);
    console.log(res)

    const fragcode =
      'varying lowp vec4 vcolor;' +
      'void main(void) {' +
      ' gl_FragColor= vcolor;' +
      '}';
    this.fragshader = this.gl.createShader(this.gl.FRAGMENT_SHADER);
    this.gl.shaderSource(this.fragshader, fragcode);
    this.gl.compileShader(this.fragshader);

    this.shaderprogram = this.gl.createProgram();
    this.gl.attachShader(this.shaderprogram, this.vertshader);
    this.gl.attachShader(this.shaderprogram, this.fragshader);
    this.gl.linkProgram(this.shaderprogram);
    this.gl.useProgram(this.shaderprogram);
  }

  test() {
    for (let i = 0; i < 700; i += 1) {
      this.vertices.push(Math.random() * 1000);
      this.vertices.push(Math.random() * 1000);
      this.colors.push(Math.random() * 255);
      this.colors.push(Math.random() * 255);
      this.colors.push(Math.random() * 255);
    }
  }

  clear_buffer() {
    this.vertices = [];
    this.colors = []
  }

  __point(p: number[], col: Color) {
    this.vertices.push(p[0]);
    this.vertices.push(p[1]);
    this.colors.push(col[0]);
    this.colors.push(col[1]);
    this.colors.push(col[2]);
  }

  triangle(p1: number[], p2: number[], p3: number[], col: Color) {
    this.__point(p1, col);
    this.__point(p2, col);
    this.__point(p3, col);
  }

  rectangle_top_bottom(top: number[], bottom: number[], col: Color) {
    const pos = [
      top,
      [top[0], bottom[1]],
      bottom,
      [bottom[0], top[1]]
    ];
    this.triangle(pos[0], pos[1], pos[2], col);
    this.triangle(pos[0], pos[2], pos[3], col);
  }

  rectangle_pos_size(pos: number[], size: number[], col: Color) {
    this.rectangle_top_bottom(pos, [pos[0] + size[0], pos[1] + size[1]], col);
  }

  point(pos: number[], col: Color, size: number = 10) {
    this.triangle(pos, [pos[0] + size, pos[1]], [pos[0], pos[1] + size], col);
  }

  line(p1: number[], p2: number[], col: Color, size = 10) {
    const dp = [p2[0] - p1[0], p2[1] - p1[1]];
    const norm = Math.sqrt(dp[0] * dp[0] + dp[1] * dp[1]);
    const e = [dp[1] * size / norm, -dp[0] * size / norm];
    const p1e = [p1[0] + e[0], p1[1] + e[1]];
    const p2e = [p2[0] + e[0], p2[1] + e[1]];
    this.triangle(p1, p2, p2e, col);
    this.triangle(p1, p2e, p2, col);
    this.triangle(p1, p2, p1e, col);
    this.triangle(p1, p1e, p2e, col);
  }

  circle(pos: number[], size: number, col: Color) {
    const dth = 6.28 / 20.0;
    for (let i = 0; i < 20; i++) {
      const p1 = [
        pos[0] + Math.cos(dth * i) * size,
        pos[1] + Math.sin(dth * i) * size
      ];
      const p2 = [
        pos[0] + Math.cos(dth * (i + 1)) * size,
        pos[1] + Math.sin(dth * (i + 1)) * size
      ];
      this.triangle(pos, p1, p2, col);
    }
  }

  wired_circle(pos: number[], size: number, col: Color, width: number) {
    const dth = 6.28 / 20.0;
    for (let i = 0; i < 20; i++) {
      const p1 = [
        pos[0] + Math.cos(dth * i) * size,
        pos[1] + Math.sin(dth * i) * size
      ];
      const p2 = [
        pos[0] + Math.cos(dth * (i + 1)) * size,
        pos[1] + Math.sin(dth * (i + 1)) * size
      ];
      this.line(p1, p2, col, width);
    }
  }

  update_buffers() {
    const matrix_data = [
      2.0 / this.width, 0.0, 0.0, 0.0,
      0.0, -2.0 / this.height, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      -1.0, 1.0, 0.0, 1.0
    ];

    // create an empty buffer object to store the vertex buffer
    // this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.vertex_buffer);
    // this.gl.bindBuffer(this.gl.ARRAY_BUFFER, null);
    // this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.color_buffer);
    // this.gl.bindBuffer(this.gl.ARRAY_BUFFER, null);
    this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.vertex_buffer);
    this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(this.vertices), this.gl.STATIC_DRAW);
    var coord = this.gl.getAttribLocation(this.shaderprogram, "vertex");
    this.gl.vertexAttribPointer(coord, 2, this.gl.FLOAT, false, 0, 0);
    this.gl.enableVertexAttribArray(coord);

    this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.color_buffer);
    this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(this.colors), this.gl.STATIC_DRAW);
    const col_attr = this.gl.getAttribLocation(this.shaderprogram, "color");
    this.gl.vertexAttribPointer(col_attr, 3, this.gl.FLOAT, false, 0, 0);
    this.gl.enableVertexAttribArray(col_attr);

    // const mat_attr = this.gl.getAttribLocation(this.shaderprogram, "view_matrix");
    const mat_attr = this.gl.getUniformLocation(this.shaderprogram, "view_matrix");
    this.gl.uniformMatrix4fv(mat_attr, false, matrix_data);
  }

  draw() {
    /*============= drawing the primitive ===============*/
    // this.gl.enable(this.gl.DEPTH_TEST);
    this.gl.clear(this.gl.COLOR_BUFFER_BIT);
    this.gl.clearColor(0., 0., 0., 1.0);
    this.gl.viewport(0, 0, this.canvas.width, this.canvas.height);
    this.gl.drawArrays(this.gl.TRIANGLES, 0, this.vertices.length / 2);
  }
}

