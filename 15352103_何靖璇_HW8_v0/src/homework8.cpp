#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tools/shader.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>
#include <deque>
#include <vector>
using namespace std;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
float computeBernstein(int i, int n, float t);
glm::vec2 computeBezierPoint(deque<glm::vec2> controlPoints, float t);
void drawLines(Shader &shader, vector<float> points, float t);

// 设置
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const unsigned int MAX_COUNT = 500;

// Bezier曲线控制点
deque<glm::vec2> controlPoints;

int main() {
	// 配置GLFW
	// --------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 创建窗口
	// --------
	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL);
	if (window == NULL) {
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// 配置GLAD
	// --------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}
	
	// 创建着色器
	// ----------
	Shader shader("shaders/bezier.vert", "shaders/bezier.frag");
	shader.use();

	// 创建计数器
	// ----------
	unsigned int count = 0;

	// 渲染循环
	// --------
	while (!glfwWindowShouldClose(window)) {
		// 处理输入
		// --------
		processInput(window);

		// 清除颜色缓冲
		// ------------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (controlPoints.size()) {
			// 计算Bezier曲线
			// --------------
			vector<float> bezierPoints;
			for (unsigned int k = 0; k <= count; k++) {
				float t = (float)k / (float)MAX_COUNT;
				glm::vec2 q = computeBezierPoint(controlPoints, t);
				bezierPoints.push_back(q.x);
				bezierPoints.push_back(q.y);
			}

			// 设置Bezier曲线VAO和VBO
			// ----------------------
			unsigned int VAO, VBO;
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bezierPoints.size(), &bezierPoints[0], GL_DYNAMIC_DRAW);

			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
			glEnableVertexAttribArray(0);

			// 绘制Bezier曲线
			// --------------
			shader.use();
			glBindVertexArray(VAO);
			glDrawArrays(GL_POINTS, 0, bezierPoints.size() / 2);

			// 绘制Bezier曲线生成过程中的直线
			// ------------------------------
			vector<float> points;
			for (unsigned int k = 0; k < controlPoints.size(); k++) {
				points.push_back(controlPoints[k].x);
				points.push_back(controlPoints[k].y);
			}
			drawLines(shader, points, (float)count / (float)MAX_COUNT);
		}

		// 更新计数器
		// ----------
		count++;
		if (count > MAX_COUNT) {
			count = 0;
		}

		// 交换缓冲和轮询事件
		// ------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

// 处理输入函数
// ------------
void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// 帧缓冲大小回调函数
// ------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
}

// 鼠标按键回调函数
// ----------------
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		// 转换坐标空间
		double x = 2 * xpos / SCR_WIDTH - 1;
		double y = 1 - 2 * ypos / SCR_HEIGHT;

		controlPoints.push_back(glm::vec2(x, y));
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		if (controlPoints.size()) {
			controlPoints.pop_back();
		}
	}
}

// 计算Bernstein基函数
// -------------------
float computeBernstein(int i, int n, float t) {
	int numerator = 1, denominator = 1;
	for (int k = n; k > n - i; k--) { // 计算阶乘分子
		numerator *= k;
	}
	for (int k = i; k > 0; k--) { // 计算阶乘分母
		denominator *= k;
	}
	float result = (float)numerator / (float)denominator * pow(t, i) * pow(1 - t, n - i);
	return result;
}

// 计算Bezier曲线Q(t)
// ------------------
glm::vec2 computeBezierPoint(deque<glm::vec2> controlPoints, float t) {
	glm::vec2 q = glm::vec2(0.0f);

	// Bezier曲线公式
	int n = controlPoints.size() - 1;
	for (int i = 0; i <= n; i++) {
		glm::vec2 p = controlPoints[i];
		float b = computeBernstein(i, n, t);
		q += p * b;
	}

	return q;
}

// 绘制Bezier曲线生成过程中的直线
// ------------------------------
void drawLines(Shader &shader, vector<float> points, float t) {
	if (points.size()) {
		// 设置直线VAO和VBO
		// ----------------
		unsigned int lineVAO, lineVBO;
		glGenVertexArrays(1, &lineVAO);
		glGenBuffers(1, &lineVBO);

		glBindVertexArray(lineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * points.size(), &points[0], GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(0);

		// 绘制直线
		// --------
		shader.use();
		glBindVertexArray(lineVAO);
		glDrawArrays(GL_LINE_STRIP, 0, points.size() / 2);

		// 计算每条直线上新的控制点
		// ------------------------
		vector<float> newPoints;
		for (unsigned int k = 0; k < points.size() - 2; k += 2) {
			float x0 = points[k];
			float y0 = points[k + 1];
			float x1 = points[k + 2];
			float y1 = points[k + 3];
			float newX = (1 - t) * x0 + t * x1;
			float newY = (1 - t) * y0 + t * y1;
			newPoints.push_back(newX);
			newPoints.push_back(newY);
		}

		// 递归绘制直线
		// ------------
		drawLines(shader, newPoints, t);
	}
}