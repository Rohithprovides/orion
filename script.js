// Initialize Feather icons
document.addEventListener('DOMContentLoaded', function() {
    feather.replace();
});

// Example programs
const examples = {
    hello: `// Hello World in Orion
fn main() {
    out("Hello, Orion World!")
    out("Fast as C, readable as Python!")
}`,
    
    fibonacci: `// Fibonacci sequence in Orion
fn fibonacci(n) {
    if n <= 1 {
        return n
    }
    return fibonacci(n - 1) + fibonacci(n - 2)
}

fn main() {
    n = 10
    out("Fibonacci sequence:")
    
    for i = 0; i < n; i = i + 1 {
        result = fibonacci(i)
        out("F(" + str(i) + ") = " + str(result))
    }
}`
};

// Load example code
function loadExample(exampleName) {
    const editor = document.getElementById('codeEditor');
    if (examples[exampleName]) {
        editor.value = examples[exampleName];
        clearOutput();
        appendOutput('Example loaded: ' + exampleName + '\n', 'info');
    }
}

// Clear editor
function clearEditor() {
    document.getElementById('codeEditor').value = '';
    clearOutput();
    appendOutput('Editor cleared.\n', 'info');
}

// Clear output
function clearOutput() {
    document.getElementById('outputArea').textContent = '';
}

// Append output with styling
function appendOutput(text, type = 'normal') {
    const outputArea = document.getElementById('outputArea');
    const span = document.createElement('span');
    
    switch(type) {
        case 'error':
            span.className = 'status-error';
            break;
        case 'success':
            span.className = 'status-success';
            break;
        case 'warning':
            span.className = 'status-warning';
            break;
        case 'info':
            span.style.color = '#6b7280';
            break;
    }
    
    span.textContent = text;
    outputArea.appendChild(span);
    outputArea.scrollTop = outputArea.scrollHeight;
}

// Set button loading state
function setButtonLoading(buttonId, loading) {
    const button = document.getElementById(buttonId);
    if (!button) return;
    
    const icon = button.querySelector('i');
    
    if (loading) {
        button.disabled = true;
        if (icon) {
            icon.setAttribute('data-feather', 'loader');
            icon.classList.add('spinner-border', 'spinner-border-sm');
        }
        feather.replace();
        
        // Add spinning animation
        const loader = button.querySelector('[data-feather="loader"]');
        if (loader) {
            loader.style.animation = 'spin 1s linear infinite';
        }
    } else {
        button.disabled = false;
        if (icon) {
            icon.classList.remove('spinner-border', 'spinner-border-sm');
            icon.setAttribute('data-feather', 'play');
            icon.style.animation = '';
        }
        feather.replace();
    }
}

// Add spinning keyframe if not already present
if (!document.querySelector('style[data-spinner]')) {
    const style = document.createElement('style');
    style.setAttribute('data-spinner', 'true');
    style.textContent = `
        @keyframes spin {
            from { transform: rotate(0deg); }
            to { transform: rotate(360deg); }
        }
    `;
    document.head.appendChild(style);
}

// Setup syntax highlighting
function setupSyntaxHighlighting() {
    // This function can be used to initialize syntax highlighting
    // For now, we'll keep it simple since we're using a textarea
    console.log('Syntax highlighting setup completed');
}

// Compile and run code
async function compileCode() {
    const code = document.getElementById('codeEditor').value.trim();
    
    if (!code) {
        appendOutput('Error: No code to compile\n', 'error');
        return;
    }
    
    // Get selected compiler
    const compilerType = document.querySelector('input[name="compiler"]:checked').value;
    const compilerLabel = compilerType === 'native' ? 'Native C++' : 'Python Interpreter';
    
    clearOutput();
    appendOutput(`Compiling with ${compilerLabel}...\n`, 'info');
    setButtonLoading('compileBtn', true);
    
    try {
        const response = await fetch('/compile', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ 
                code: code,
                compiler: compilerType
            })
        });
        
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}: ${response.statusText}`);
        }
        
        const result = await response.json();
        
        if (result.success) {
            appendOutput('✓ Compilation successful!\n', 'success');
            appendOutput('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n', 'info');
            appendOutput('Program Output:\n', 'info');
            appendOutput(result.output + '\n', 'normal');
            appendOutput('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n', 'info');
            appendOutput(`✓ Execution completed (${result.execution_time}ms)\n`, 'success');
        } else {
            appendOutput('✗ Compilation failed!\n', 'error');
            appendOutput('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n', 'error');
            appendOutput('Errors:\n', 'error');
            appendOutput(result.error + '\n', 'error');
        }
    } catch (error) {
        console.error('Compilation error:', error);
        appendOutput('✗ Network error: ' + error.message + '\n', 'error');
    } finally {
        setButtonLoading('compileBtn', false);
    }
}

// Check syntax
async function checkSyntax() {
    const code = document.getElementById('codeEditor').value.trim();
    
    if (!code) {
        appendOutput('Error: No code to check\n', 'error');
        return;
    }
    
    clearOutput();
    appendOutput('Checking syntax...\n', 'info');
    
    try {
        const response = await fetch('/check-syntax', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ code: code })
        });
        
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}: ${response.statusText}`);
        }
        
        const result = await response.json();
        
        if (result.valid) {
            appendOutput('✓ Syntax is valid!\n', 'success');
            appendOutput('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n', 'info');
            appendOutput('Tokens found:\n', 'info');
            result.tokens.forEach(token => {
                appendOutput(`  ${token.type}: ${token.value}\n`, 'normal');
            });
        } else {
            appendOutput('✗ Syntax errors found!\n', 'error');
            appendOutput('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n', 'error');
            appendOutput(result.error + '\n', 'error');
        }
    } catch (error) {
        console.error('Syntax check error:', error);
        appendOutput('✗ Error checking syntax: ' + error.message + '\n', 'error');
    }
}

// Show AST
async function showAST() {
    const code = document.getElementById('codeEditor').value.trim();
    
    if (!code) {
        appendOutput('Error: No code to parse\n', 'error');
        return;
    }
    
    clearOutput();
    appendOutput('Generating Abstract Syntax Tree...\n', 'info');
    
    try {
        const response = await fetch('/ast', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ code: code })
        });
        
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}: ${response.statusText}`);
        }
        
        const result = await response.json();
        
        if (result.success) {
            appendOutput('✓ AST generated successfully!\n', 'success');
            appendOutput('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n', 'info');
            appendOutput('Abstract Syntax Tree:\n', 'info');
            appendOutput(result.ast + '\n', 'normal');
        } else {
            appendOutput('✗ Failed to generate AST!\n', 'error');
            appendOutput('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n', 'error');
            appendOutput(result.error + '\n', 'error');
        }
    } catch (error) {
        console.error('AST generation error:', error);
        appendOutput('✗ Error generating AST: ' + error.message + '\n', 'error');
    }
}

// Keyboard shortcuts
document.addEventListener('keydown', function(e) {
    if (e.ctrlKey || e.metaKey) {
        switch(e.key) {
            case 'Enter':
                e.preventDefault();
                compileCode();
                break;
            case 'k':
                e.preventDefault();
                clearOutput();
                break;
        }
    }
});

// Auto-resize text area
document.getElementById('codeEditor').addEventListener('input', function() {
    // Simple auto-indentation
    if (event.inputType === 'insertLineBreak') {
        const textarea = event.target;
        const lines = textarea.value.split('\n');
        const currentLineIndex = lines.length - 2;
        const currentLine = lines[currentLineIndex];
        
        // Basic indentation logic
        let indent = '';
        if (currentLine.includes('{')) {
            const match = currentLine.match(/^(\s*)/);
            indent = match ? match[1] + '    ' : '    ';
        } else {
            const match = currentLine.match(/^(\s*)/);
            indent = match ? match[1] : '';
        }
        
        if (indent) {
            const cursorPosition = textarea.selectionStart;
            const textBefore = textarea.value.substring(0, cursorPosition);
            const textAfter = textarea.value.substring(cursorPosition);
            textarea.value = textBefore + indent + textAfter;
            textarea.setSelectionRange(cursorPosition + indent.length, cursorPosition + indent.length);
        }
    }
});

// Initialize when page loads
document.addEventListener('DOMContentLoaded', function() {
    loadExample('hello');
    setupSyntaxHighlighting();
    
    // Add event listeners to prevent unhandled promise rejections
    window.addEventListener('unhandledrejection', function(event) {
        console.error('Unhandled promise rejection:', event.reason);
        event.preventDefault();
    });
});
