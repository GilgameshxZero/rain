#include <rain.hpp>

int main() {
	using namespace Rain;
	using Color = Console::Color;

	Console::cout({.fgColor = Color::BLACK}, "BLACK\n");
	Console::cout({.fgColor = Color::RED}, "RED\n");
	Console::cout({.fgColor = Color::GREEN}, "GREEN\n");
	Console::cout({.fgColor = Color::YELLOW}, "YELLOW\n");
	Console::cout({.fgColor = Color::BLUE}, "BLUE\n");
	Console::cout({.fgColor = Color::MAGENTA}, "MAGENTA\n");
	Console::cout({.fgColor = Color::CYAN}, "CYAN\n");
	Console::cout({.fgColor = Color::WHITE}, "WHITE\n");
	Console::cout(
		{.fgColor = Color::BLACK, .fgIntense = true}, "BLACK INTENSE\n");
	Console::cout({.fgColor = Color::RED, .fgIntense = true}, "RED INTENSE\n");
	Console::cout(
		{.fgColor = Color::GREEN, .fgIntense = true}, "GREEN INTENSE\n");
	Console::cout(
		{.fgColor = Color::YELLOW, .fgIntense = true}, "YELLOW INTENSE\n");
	Console::cout({.fgColor = Color::BLUE, .fgIntense = true}, "BLUE INTENSE\n");
	Console::cout(
		{.fgColor = Color::MAGENTA, .fgIntense = true}, "MAGENTA INTENSE\n");
	Console::cout({.fgColor = Color::CYAN, .fgIntense = true}, "CYAN INTENSE\n");
	Console::cout(
		{.fgColor = Color::WHITE, .fgIntense = true}, "WHITE INTENSE\n");
	Console::cout(
		{.fgColor = Color::BLACK, .bgColor = Color::YELLOW, .bgIntense = true},
		"BLACK\n");
	Console::cout(
		{.fgColor = Color::RED, .bgColor = Color::YELLOW, .bgIntense = true},
		"RED\n");
	Console::cout(
		{.fgColor = Color::GREEN, .bgColor = Color::YELLOW, .bgIntense = true},
		"GREEN\n");
	Console::cout(
		{.fgColor = Color::YELLOW, .bgColor = Color::YELLOW, .bgIntense = true},
		"YELLOW\n");
	Console::cout(
		{.fgColor = Color::BLUE, .bgColor = Color::YELLOW, .bgIntense = true},
		"BLUE\n");
	Console::cout(
		{.fgColor = Color::MAGENTA, .bgColor = Color::YELLOW, .bgIntense = true},
		"MAGENTA\n");
	Console::cout(
		{.fgColor = Color::CYAN, .bgColor = Color::YELLOW, .bgIntense = true},
		"CYAN\n");
	Console::cout(
		{.fgColor = Color::WHITE, .bgColor = Color::YELLOW, .bgIntense = true},
		"WHITE\n");

	Console::cout({.row = 1, .col = 4}, "POSITION");
	return 0;
}
