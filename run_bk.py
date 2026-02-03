import chess
import chess.engine

# --- CONFIGURATION ---
# Replace this with the actual name of your engine executable
engine_path = "./build/chess_engine"  
epd_file = "bk_test.epd"
time_per_move = 2.0  # Seconds per position (Standard BK is often 15s or 30s)
# ---------------------

def run_test():
    try:
        engine = chess.engine.SimpleEngine.popen_uci(engine_path)
    except FileNotFoundError:
        print(f"Error: Could not find engine at {engine_path}")
        return

    solved = 0
    total = 0

    print(f"Starting Bratko-Kopec Test on {engine_path}...")
    print(f"Time limit: {time_per_move} seconds per position.\n")

    with open(epd_file) as f:
        # Loop through each line in the EPD file
        for line in f:
            if not line.strip(): continue
            
            # Parse the EPD line
            board, ops = chess.Board.from_epd(line)
            total += 1
            
            # Get the ID and the Best Move (bm) from the EPD
            pos_id = ops.get("id", f"#{total}")
            best_moves = ops.get("bm", []) # returns a list of Move objects
            
            if not best_moves:
                print(f"Skipping {pos_id}: No best move defined in EPD.")
                continue

            # Ask the engine to search
            result = engine.play(board, chess.engine.Limit(time=time_per_move))
            
            # Check if the engine's move matches the EPD's best move
            if result.move in best_moves:
                print(f"[PASS] {pos_id}: Found {result.move}")
                solved += 1
            else:
                # Convert the expected moves to algebraic notation for display
                bm_str = ", ".join([str(m) for m in best_moves])
                print(f"[FAIL] {pos_id}: Engine played {result.move}, expected {bm_str}")

    print("\n------------------------------------------------")
    print(f"Final Score: {solved}/{total}")
    print("------------------------------------------------")
    
    engine.quit()

if __name__ == "__main__":
    run_test()
