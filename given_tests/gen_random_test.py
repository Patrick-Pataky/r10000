#!/usr/bin/env python3
import argparse
import random
import os
import json

def generate_instruction(registers):
	instr_types = ["add", "addi", "sub", "mulu", "divu", "remu"]
	instr = random.choice(instr_types)
	
	if instr == "addi":
		dest = random.choice(registers)
		opA = random.choice(registers)
		imm = random.randint(-50, 50)
		return f"addi {dest}, {opA}, {imm}"
	elif instr in ["add", "sub", "mulu"]:
		dest = random.choice(registers)
		opA = random.choice(registers)
		opB = random.choice(registers)
		return f"{instr} {dest}, {opA}, {opB}"
	elif instr in ["divu", "remu"]:
		# For these instructions, only divu and remu, we want to occasionally
		# force the second operand to be x0 to trigger division-by-0.
		dest = random.choice(registers)
		opA = random.choice(registers)
		if random.random() < 0.1:  # 10% chance to use x0
			opB = "x0"
		else:
			opB = random.choice(registers)
		return f"{instr} {dest}, {opA}, {opB}"
	else:
		raise ValueError("Unexpected instruction type.")

def generate_program(registers, max_num):
	num_instructions = random.randint(1, max_num)
	program = []
	for _ in range(num_instructions):
		instr = generate_instruction(registers)
		program.append(instr)
	return program

def main():
	parser = argparse.ArgumentParser(description="Generate N random programs.")
	parser.add_argument("N", type=int, help="Number of programs to generate")
	parser.add_argument("--seed", type=int, default=42, help="Random seed for reproducibility")
	args = parser.parse_args()
	
	random.seed(args.seed)
	
	registers = [f"x{i}" for i in range(32)]

	START_TEST=14	
	MAX_NUM_INSTR=100
	
	for i in range(START_TEST, START_TEST + args.N):
		program = generate_program(registers, MAX_NUM_INSTR)

		os.makedirs(str(i), exist_ok=True)
		filename = os.path.join(str(i), f"input.json")

		with open(filename, "w") as f:
			json.dump(program, f, indent=2)
		
		desc = f"random program of {len(program)} instructions"
		with open(os.path.join(str(i), "desc.txt"), "w") as f:
			f.write(desc)

		print(f"Generated {filename} with {len(program)} instructions.")

if __name__ == "__main__":
	main()
