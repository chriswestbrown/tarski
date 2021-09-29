/* This code was created by using the tutorial at https://docs.oracle.com/javase/tutorial/uiswing/components/textfield.html. */

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

public class JTarski extends JPanel implements ActionListener {
    protected JTextField textField;
    protected JTextArea textArea;
    private final static String newline = "\n";

    protected static int numcells = 50000000;
    protected static int timeout = 5;

    public JTarski() {
        super(new GridBagLayout());

        textField = new JTextField(30);
        textField.addActionListener(this);

        textArea = new JTextArea(10, 30);
        textArea.setEditable(false);
        JScrollPane scrollPane = new JScrollPane(textArea);

        //Add Components to this panel.
        GridBagConstraints c = new GridBagConstraints();
        c.gridwidth = GridBagConstraints.REMAINDER;

        c.fill = GridBagConstraints.BOTH;
        c.weightx = 1.0;
        c.weighty = 1.0;
        add(scrollPane, c);

        c.fill = GridBagConstraints.HORIZONTAL;
        add(textField, c);

        //Initialization
        textArea.append("Welcome to JTarski!" + newline);
        textArea.append("Numcells are set to " + numcells + ", timeout is " + timeout + "." + newline);
        textField.setText("(qepcad-api-call [ex x [x > 0]])");
    }

    public void actionPerformed(ActionEvent evt) {
        String text = textField.getText();
        String output = tarski.PCLIB(text);

        textArea.append("> " + text + newline);
        textArea.append(output);
        textField.selectAll();

        //Make sure the new text is visible, even if there
        //was a selection in the text area.
        textArea.setCaretPosition(textArea.getDocument().getLength());
    }

    /**
     * Create the GUI and show it.  For thread safety,
     * this method should be invoked from the
     * event dispatch thread.
     */
    private static void createAndShowGUI() {
        //Create and set up the window.
        JFrame frame = new JFrame("JTarski");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        //Add contents to the window.
        frame.add(new JTarski());

        //Display the window.
        frame.pack();
        frame.setVisible(true);
        //Center the window.
        frame.setLocationRelativeTo(null);
    }

    public static void main(String[] args) {
        System.loadLibrary("tarski");
        tarski.mainLIB(numcells, timeout);

        //Schedule a job for the event dispatch thread:
        //creating and showing this application's GUI.
        javax.swing.SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                createAndShowGUI();
            }
        });
    }
}
