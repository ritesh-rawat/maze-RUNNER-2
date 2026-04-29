// Wait for the WASM module (maze.js) to be ready
window.Module = {};
Module.onRuntimeInitialized = () => {
    console.log("WASM Runtime Initialized.");

    // --- C Function Wrappers ---
    const initMaze = Module._init_maze;
    const runGeneration = Module._run_generation;
    const runSolver = Module._run_solver;
    const getMazeBuffer = Module._get_maze_buffer;

    // --- HTML Element Refs ---
    const slider = document.getElementById('size-slider');
    const sizeValue = document.getElementById('size-value');
    const btnGenerate = document.getElementById('btn-generate');
    const btnSolve = document.getElementById('btn-solve');
    const solveControls = document.getElementById('solve-controls');
    const statusText = document.getElementById('status-text');
    const canvas = document.getElementById('maze-canvas');
    const ctx = canvas.getContext('2d');

    // --- App State ---
    let N = 0; // Current maze size
    let mazeDataPtr = 0;
    let mazeArray = null;

    // --- ASCII codes for chars ---
    // !!! FIX: Ye lines ab 'COLORS' se pehle hain !!!
    const WALL = 35; // '#'
    const PATH = 32; // ' '
    const BFS_PATH = 42; // '*'
    const DFS_PATH = 46; // '.'

    // --- Colors (Naya Look) ---
    const COLORS = {
        [WALL]: '#222',     // Wall (thoda halka)
        [PATH]: '#e0e0e0',  // Path
        [BFS_PATH]: '#0099ff', // BFS Solution (Blue)
        [DFS_PATH]: '#ff55a3'  // DFS Solution (Pink)
    };


    // --- Main Drawing Function ---
    function drawMaze() {
        if (N === 0) return;

        // Get the maze data from C/WASM memory
        mazeDataPtr = getMazeBuffer();
        mazeArray = new Uint8Array(Module.HEAPU8.buffer, mazeDataPtr, N * N);

        // Calculate cell size
        const displaySize = Math.min(window.innerHeight * 0.8, window.innerWidth * 0.6);
        const cellSize = Math.floor(displaySize / N);
        canvas.width = cellSize * N;
        canvas.height = cellSize * N;

        ctx.clearRect(0, 0, canvas.width, canvas.height);

        // Loop through the 1D array as if it were 2D
        for (let y = 0; y < N; y++) {
            for (let x = 0; x < N; x++) {
                const charCode = mazeArray[y * N + x];
                ctx.fillStyle = COLORS[charCode] || COLORS[WALL];
                ctx.fillRect(x * cellSize, y * cellSize, cellSize, cellSize);
            }
        }
    }

    // --- Event Listeners ---
    slider.oninput = (e) => {
        sizeValue.textContent = e.target.value;
    };

    btnGenerate.onclick = () => {
        N = parseInt(slider.value, 10);
        statusText.textContent = `Generating ${N}x${N} maze...`;
        
        btnGenerate.disabled = true;
        btnSolve.disabled = true;

        setTimeout(() => { 
            initMaze(N);
            runGeneration();
            drawMaze();
            
            statusText.textContent = `Generated ${N}x${N} maze.`;
            solveControls.style.display = 'block';
            btnGenerate.disabled = false;
            btnSolve.disabled = false;
        }, 50);
    };

    btnSolve.onclick = () => {
        const solverType = document.querySelector('input[name="solver"]:checked').value;
        statusText.textContent = "Solving...";
        btnGenerate.disabled = true;
        btnSolve.disabled = true;

        setTimeout(() => {
            const found = runSolver(parseInt(solverType, 10));
            
            if (found) {
                statusText.textContent = "Path found!";
            } else {
                statusText.textContent = "No path found!";
            }
            drawMaze(); 
            btnGenerate.disabled = false;
            btnSolve.disabled = false;
        }, 50);
    };

    // Initial value
    sizeValue.textContent = slider.value;
};