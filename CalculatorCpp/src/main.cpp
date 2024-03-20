#include <stdio.h>
#include <iostream>
#include <string>
#include <unordered_set>
#include <numbers>
#include <sstream>
#include <cstdlib>
#include <format>

#include "lexer.h"
#include "parser.h"
#include "evaluation.h"
#include "initialization.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "imgui_impl_win32.h"
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


	// Initialize GLFW
	if (!glfwInit())
		return 1;

	// Create window with graphics context
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	std::string expression = "";
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
		// Create a expression evaluator view port
		ImGui::Begin("Calculator", nullptr);
		{
			ImGui::InputText("Expression", &expression);

			if (ImGui::Button("Calculate"))
			{
				auto lexResult = lex.lexing(expression);
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

						if (auto result{ eval.evaluateExpressionTree(root.getValue()) }; !result.isError()) {
							resultText = std::format("Result: {}", std::to_string(result.getValue()));
						}
						else
							resultText = "ERROR: " + std::string(result.getException().what());

						Parser::freeOperatorTree(root.getValue());
					}
					else {
						resultText = "ERROR: " + std::string(root.getException().what());
					}
				}
			}
		}
		ImGui::End();

		// Create a result view port
		ImGui::Begin("Result", nullptr);
		{
			ImGui::Text(lexResultText.c_str());
			ImGui::Text(parsedResultText.c_str());
			ImGui::Text(operationTreeText.c_str());
			ImGui::Text(resultText.c_str());
		}
		ImGui::End();

		// Create a numPad view port
		ImGui::Begin("Numpad", nullptr);
		{
			const float total_viewport_width = ImGui::GetContentRegionAvail().x;
			const float number_spacing = 10.0f;
			float number_element_width = (total_viewport_width - number_spacing * 2) / 3.0f;

			// add 3x3 num-pad
			for (int i = 1; i < 10; i++) {
				if (ImGui::Button(std::to_string(i).c_str(), ImVec2(number_element_width, 30)))
					expression += std::to_string(i);

				if (i % 3 != 0) ImGui::SameLine(0, number_spacing);
				else ImGui::Spacing();
			}

			// add entire viewport width "0" button
			if (ImGui::Button("0", ImVec2(total_viewport_width, 30)))
				expression += '0';
		}
		ImGui::End();

		// Create a OperatorPad view port
		ImGui::Begin("OperatorPad", nullptr);
		{
			const float total_viewport_width = ImGui::GetContentRegionAvail().x;
			const float operator_spacing = 10.0f;
			const size_t element_amount = 5;
			const float element_width = (total_viewport_width - operator_spacing * element_amount) / static_cast<float>(element_amount);

			// start adding the operator buttons, (5 cols per row)
			int index = 0;
			for (const auto& ii : lex.getKeywords())
			{
				if (ImGui::Button(ii.c_str(), ImVec2(element_width, 30)))
					expression += ii;
				if ((++index) % element_amount != 0) ImGui::SameLine(0, operator_spacing);
				else ImGui::Spacing();
			}
		}

		ImGui::End();

		// Rendering
		ImGui::Render();

		int display_w;
		int display_h;

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

	return 0;
}