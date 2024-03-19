#include <stdio.h>
#include <iostream>
#include <string>
#include <unordered_set>
#include <numbers>
#include <sstream>

#include "lexer.h"
#include "parser.h"
#include "evaluation.h"
#include "initialization.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"

#define DEBUG
#include "debug.cpp"

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main()
{
	Lexer lex;
	initializeLexer(lex);

	Parser pas;
	initializeParser(pas);

	Evaluate<long double> eval(pas);
	initializeEvaluator(eval);

	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	// Decide GL+GLSL versions
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create window with graphics context
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
	if (window == nullptr)
		return EXIT_FAILURE;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	static char expr[256] = "";
	static float result = 0.0f;
	std::string lexResultText;
	std::string parsedResultText;
	std::string operationTreeText;
	std::string resultText;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Calculator GUI
		ImGui::Begin("Calculator", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		ImGui::InputText("Expression", expr, sizeof(expr));

		if (ImGui::Button("Calculate"))
		{
			auto lexResult = lex.lexing(expr);
			std::stringstream lexResultStr;
			lexResultStr << lexResult;
			lexResultText = "Lexing: " + lexResultStr.str();

			auto parsedResult = pas.parseNumbers(lexResult);
			std::stringstream parsedResultStr;
			parsedResultStr << lexResult;
			parsedResultText = "Parsing Number: " + parsedResultStr.str();

			if (!pas.parserReady().has_value()) {
				auto root = pas.createOperatorTree(parsedResult);

				if (!root.isError()) {
					operationTreeText = "Operation Tree: " + pas.printOpertatorTree(root.getValue());

					if (auto result{ eval.evaluateExpressionTree(root.getValue()) }; !result.isError())
						resultText = "Result: " + std::to_string(result.getValue());
					else
						resultText = "ERROR: " + std::string(result.getException().what());

					Parser::freeOperatorTree(root.getValue());
				}
				else {
					resultText = "ERROR: " + std::string(root.getException().what());
				}
			}
		}

		ImGui::Text(lexResultText.c_str());
		ImGui::Text(parsedResultText.c_str());
		ImGui::Text(operationTreeText.c_str());
		ImGui::Text(resultText.c_str());

		ImGui::End();

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glfwSwapBuffers(window);
	}


	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();



	size_t count{ 0 };
	while (++count)
	{
		std::cout << "\n## EXPRESSION: " << count << "##\n";
		std::cout << "Expression = ";
		std::string input{};

		std::getline(std::cin, input);

		if (input == "quit")
			return 0;

		auto lexResult = lex.lexing(input);
		std::cout << "Lexing: " << lexResult << "\n";

		auto parsedResult = pas.parseNumbers(lexResult);
		std::cout << "Parsing Number: " << parsedResult << "\n";

		if (!pas.parserReady().has_value()) {
			auto root = pas.createOperatorTree(parsedResult);

			if (!root.isError()) {
				std::cout << "Operation Tree: " << pas.printOpertatorTree(root.getValue()) << "\n";

				if (auto result{ eval.evaluateExpressionTree(root.getValue()) }; !result.isError())
					std::cout << "Result: " << result.getValue() << "\n";
				else
					std::cout << "ERROR: " << result.getException().what() << "\n";

				Parser::freeOperatorTree(root.getValue());
			}
			else {
				std::cout << "ERROR: " << root.getException().what() << "\n";
			}
		}
	}
	
	return 0;
}