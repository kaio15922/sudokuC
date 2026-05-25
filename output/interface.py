import customtkinter as ctk
import subprocess
import os
import threading

ctk.set_appearance_mode("Dark") # Força o modo escuro para melhor legibilidade
ctk.set_default_color_theme("blue")

class SudokuInterface(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title("Sudoku solve")
        self.geometry("1020x600")
        self.resizable(False, False)

        # Guarda a referência do processo em C que vai ficar rodando
        self.processo_c = None 

        # --- TÍTULO ---
        self.title_label = ctk.CTkLabel(self, text="Buscador de Soluções Sudoku ", font=ctk.CTkFont(size=24, weight="bold"))
        self.title_label.pack(padx=20, pady=20)

        # --- STATUS DE CARREGAMENTO ---
        self.status_label = ctk.CTkLabel(self, text="Iniciando arquivo .c e ordenando dados...", text_color="#003CFF", font=ctk.CTkFont(size=14, weight="bold"))
        self.status_label.pack(padx=20, pady=(10,0))
        
        self.progress_bar = ctk.CTkProgressBar(self, width=450)
        self.progress_bar.pack(padx=20, pady=(2,10))
        self.progress_bar.start() # Barra fica animando enquanto carrega

        # --- INPUT DO PUZZLE (Inicialmente desabilitado) ---
        self.input_label = ctk.CTkLabel(self, text="Insira a string do Puzzle (81 caracteres):", font=ctk.CTkFont(size=14))
        self.input_label.pack(padx=20, pady=(20,0))
        
        self.puzzle_entry = ctk.CTkEntry(self, width=600, placeholder_text="Aguardando carregamento...", state="disabled")
        self.puzzle_entry.pack(padx=20, pady=10)

        # --- BOTÃO DE BUSCA (Inicialmente desabilitado) ---
        self.search_button = ctk.CTkButton(self, text="Buscar Solução", command=self.enviar_para_c, state="disabled", font=ctk.CTkFont(size=14, weight="bold"))
        self.search_button.pack(padx=25, pady=(5,40))

        # --- ÁREA DE RESULTADO ---
        self.solucao_text = ctk.CTkLabel(self, text="Resultado da busca:", font=ctk.CTkFont(size=13))
        self.solucao_text.pack(padx = 2, pady = (10,0))
       
        self.solucao_output = ctk.CTkTextbox(self, width=600, height=50,wrap="none",state="disabled", fg_color="#2A2A2A", text_color="#00FF00", font=ctk.CTkFont(size=13))
        self.solucao_output.pack(padx=20, pady=(5,40)) 
        
        # --- ÁREA DO RELATÓRIO ---
        self.relatorio_label = ctk.CTkLabel(self, text="Tempo de Busca Binária no C:", font=ctk.CTkFont(size=13))
        self.relatorio_label.pack(padx=20, pady=(10,0))
        
        self.relatorio_textbox = ctk.CTkTextbox(self, width=600, height=160, state="disabled", fg_color="#1E1E1E", text_color="#FFFFFF")
        self.relatorio_textbox.pack(padx=20, pady=10)

        # Dispara o carregamento do C em uma Thread separada para a interface não travar
        threading.Thread(target=self.iniciar_arquivo_c, daemon=True).start()
        
        # Garante que o processo em C fecha quando fecharmos a janela do Python
        self.bind("<Destroy>", self.finalizar_programa)

    def iniciar_arquivo_c(self):
        #precisa saber qual o OS do usuário:
        caminho_exe = "./susu.exe" if os.name != 'nt' else "susu.exe"
        try:
            # Força o Python a rodar exatamente na pasta onde o executável está
            pasta_do_exe = os.path.dirname(os.path.abspath(__file__))

            self.processo_c = subprocess.Popen(
                [os.path.join(pasta_do_exe, caminho_exe)],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                encoding='utf-8',
                cwd=pasta_do_exe # Define a pasta de trabalho correta!
            )

            #loop responsável por verificar se a leitura do arquivo .c deu certo
            while True:
                linha = self.processo_c.stdout.readline().strip()
                if "ERRO_ARQUIVO" in linha:
                    self.status_label.configure(text="Erro: O arquivo sudoku.csv não foi encontrado!", text_color="red")
                    self.progress_bar.stop()
                    return
                if "PRONTO" in linha:
                    break
            
            #Carregamento e ordenação de dados deu certo

            #Aqui, faz a barra de carregamento sumir
            self.status_label.configure(text="Dados carregados e ordenados com sucesso!", text_color="#00FF00")
            self.progress_bar.stop()
            self.progress_bar.pack_forget()
            
            #Desbloqueia a caixinha pra por o puzzle:
            self.puzzle_entry.configure(state="normal", placeholder_text="Ex: 00676767...")
            self.search_button.configure(state="normal")

        except Exception as e:
            self.status_label.configure(text=f"Erro ao iniciar o arquivo .C: {str(e)}", text_color="red")
            self.progress_bar.stop()

    def enviar_para_c(self):
        puzzle_texto = self.puzzle_entry.get().strip()
        if not puzzle_texto:
            return

        try:
            # Envia o puzzle para o processo em C
            self.processo_c.stdin.write(puzzle_texto + "\n")
            self.processo_c.stdin.flush()

            # Lê exatamente as 4 linhas estruturadas que o C devolve
            linha_solucao = self.processo_c.stdout.readline().strip()
            linha_t_carrega = self.processo_c.stdout.readline().strip()
            linha_t_ordena = self.processo_c.stdout.readline().strip()
            linha_t_busca = self.processo_c.stdout.readline().strip()

            #Tira qualquer caractere indesejado (Só número)
            if "ERRO" in linha_solucao:
                linha_solucao = "Puzzle não encontrado."
                linha_t_busca = "0.000000"
            
            # Monta o Relatório de Runtime completo e elegante
            texto_relatorio = (
                "==============================================================\n"
                "                     RELATÓRIO DE RUNTIME                     \n"
                "==============================================================\n"
                f"       Tempo de Carregamento (.csv):      {float(linha_t_carrega):.6f} segundos\n"
                f"       Tempo de Ordenação (Quicksort):    {float(linha_t_ordena):.6f} segundos\n"
                f"       Tempo de Busca (Busca Binária):    {float(linha_t_busca):.6f} segundos\n"
                f"       Tempo Total do Processo C:         {float(linha_t_carrega)+float(linha_t_ordena)+float(linha_t_busca):.6f} segundos\n"
                "=============================================================="
            )
            
            self.atualizar_campos(linha_solucao, texto_relatorio)

        except Exception as e:
            self.atualizar_campos("ERRO", f"Falha na comunicação com o arquivo .C: {str(e)}")

    def atualizar_campos(self, solucao, relatorio):
        # Atualiza a caixa da solução
        self.solucao_output.configure(state="normal")
        self.solucao_output.delete("1.0", ctk.END) # Mudou de 0 para "1.0"
        self.solucao_output.insert("1.0", solucao) # Mudou de 0 para "1.0"
        self.solucao_output.configure(state="disabled")

        # Atualiza o bloco do relatório
        self.relatorio_textbox.configure(state="normal")
        self.relatorio_textbox.delete("1.0", ctk.END)
        self.relatorio_textbox.insert("1.0", relatorio)
        self.relatorio_textbox.configure(state="disabled")

    def finalizar_programa(self, event):
        if self.processo_c:
            self.processo_c.kill() # Mata o C ao fechar a janela

if __name__ == "__main__":
    app = SudokuInterface()
    app.mainloop()