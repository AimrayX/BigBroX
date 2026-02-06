import chess
import chess.engine

# PATHS TO YOUR ENGINES
ENGINE_V8_PATH = "./Versions/BigBroX_V8"
ENGINE_LATEST_PATH = "./build/chess_engine"
STOCKFISH_PATH = "stockfish" # Or path to stockfish executable

def play_match():
    # 1. Initialize Engines
    engine1 = chess.engine.SimpleEngine.popen_uci(ENGINE_V8_PATH)
    engine2 = chess.engine.SimpleEngine.popen_uci(ENGINE_LATEST_PATH)
    sf = chess.engine.SimpleEngine.popen_uci(STOCKFISH_PATH)

    board = chess.Board()
    
    print(f"{'Move':<5} | {'V8 Eval':<10} | {'Latest Eval':<10} | {'Stockfish (Truth)':<10}")
    print("-" * 50)

    while not board.is_game_over():
        # Determine whose turn it is
        current_engine = engine1 if board.turn == chess.WHITE else engine2
        
        # 2. Get the engine's move
        result = current_engine.play(board, chess.engine.Limit(time=1.0))

        # --- FIX STARTS HERE ---
        # Safely get evaluation. If missing, use a dummy "0.00" PovScore.
        # We need a PovScore (Point of View Score) to call .white() on it.
        default_score = chess.engine.PovScore(chess.engine.Cp(0), chess.WHITE)
        score_obj = result.info.get("score", default_score)
        
        # Check if it's a mate score or centipawns
        if score_obj.is_mate():
            move_eval = f"Mate {score_obj.white().mate()}"
        else:
            move_eval = score_obj.white().score()
        # --- FIX ENDS HERE ---

        # 3. Ask Stockfish for the "Real" evaluation
        sf_info = sf.analyse(board, chess.engine.Limit(depth=18))
        sf_eval = sf_info["score"].white().score()

        # 4. Print Comparison
        print(f"{board.fullmove_number}. {str(result.move):<5} | {str(move_eval):<10} | {str(sf_eval)}")
        
        board.push(result.move)

    engine1.quit()
    engine2.quit()
    sf.quit()

if __name__ == "__main__":
    play_match()
